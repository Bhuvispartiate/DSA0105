#include "VaultWriter.hpp"
#include "VaultFormat.hpp"
#include "Crypto.hpp"
#include "FileUtils.hpp"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace SecureVault {

VaultWriter::VaultWriter(const std::string& vaultPath, const std::string& passphrase)
    : m_vaultPath(vaultPath),
      m_passphrase(passphrase),
      m_iterations(DEFAULT_PBKDF2_ITERATIONS) {}

VaultWriter::~VaultWriter() {
    Crypto::secureZero(m_passphrase.data(), m_passphrase.size());
}

void VaultWriter::setKdfIterations(uint32_t iterations) {
    if (iterations < 1000) {
        throw std::invalid_argument("KDF iterations cannot be less than 1,000.");
    }
    m_iterations = iterations;
}

void VaultWriter::addSource(const std::string& path) {
    m_sources.push_back(path);
}

void VaultWriter::addSources(const std::vector<std::string>& paths) {
    m_sources.insert(m_sources.end(), paths.begin(), paths.end());
}

std::vector<uint8_t> VaultWriter::serializePayload(const std::vector<VaultEntry>& entries) {
    // Calculate total required buffer size
    size_t totalSize = sizeof(uint32_t); // entry count
    for (const auto& entry : entries) {
        totalSize += sizeof(uint32_t) + entry.relativePath.size();
        totalSize += sizeof(uint64_t); // fileSize
        totalSize += sizeof(uint64_t); // modifiedTime
        totalSize += 32;               // sha256
        totalSize += entry.data.size();
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(totalSize);

    auto appendUint32 = [&buffer](uint32_t val) {
        uint8_t b[4] = {
            static_cast<uint8_t>(val >> 24),
            static_cast<uint8_t>(val >> 16),
            static_cast<uint8_t>(val >> 8),
            static_cast<uint8_t>(val)
        };
        buffer.insert(buffer.end(), b, b + 4);
    };

    auto appendUint64 = [&buffer](uint64_t val) {
        uint8_t b[8] = {
            static_cast<uint8_t>(val >> 56),
            static_cast<uint8_t>(val >> 48),
            static_cast<uint8_t>(val >> 40),
            static_cast<uint8_t>(val >> 32),
            static_cast<uint8_t>(val >> 24),
            static_cast<uint8_t>(val >> 16),
            static_cast<uint8_t>(val >> 8),
            static_cast<uint8_t>(val)
        };
        buffer.insert(buffer.end(), b, b + 8);
    };

    appendUint32(static_cast<uint32_t>(entries.size()));

    for (const auto& entry : entries) {
        appendUint32(static_cast<uint32_t>(entry.relativePath.size()));
        buffer.insert(buffer.end(), entry.relativePath.begin(), entry.relativePath.end());
        appendUint64(entry.fileSize);
        appendUint64(entry.modifiedTime);
        buffer.insert(buffer.end(), entry.sha256.begin(), entry.sha256.end());
        if (!entry.data.empty()) {
            buffer.insert(buffer.end(), entry.data.begin(), entry.data.end());
        }
    }

    return buffer;
}

PackStats VaultWriter::pack(ProgressCallback callback) {
    if (m_passphrase.empty()) {
        throw std::runtime_error("Encryption key/passphrase cannot be empty.");
    }
    if (m_sources.empty()) {
        throw std::runtime_error("No input files or directories specified for vault.");
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    auto fileItems = FileUtils::collectFiles(m_sources);
    if (fileItems.empty()) {
        throw std::runtime_error("No valid regular files found to package.");
    }

    std::vector<VaultEntry> entries;
    entries.reserve(fileItems.size());

    uint64_t totalOriginalBytes = 0;

    for (size_t i = 0; i < fileItems.size(); ++i) {
        const auto& item = fileItems[i];
        if (callback) {
            callback(item.vaultRelativePath, i + 1, fileItems.size());
        }

        VaultEntry entry;
        entry.relativePath = item.vaultRelativePath;
        entry.fileSize = item.fileSize;
        entry.modifiedTime = item.modifiedTime;
        entry.data = FileUtils::readFile(item.diskPath);
        entry.sha256 = Crypto::SHA256::hash(entry.data);

        totalOriginalBytes += entry.fileSize;
        entries.push_back(std::move(entry));
    }

    // 1. Serialize all entries to plaintext binary blob
    std::vector<uint8_t> plaintext = serializePayload(entries);

    // Free file memory from entries
    entries.clear();

    // 2. Generate random Salt and IV
    auto salt = Crypto::getRandomBytes(SALT_SIZE);
    auto iv = Crypto::getRandomBytes(IV_SIZE);

    // 3. Derive 64 bytes (32 bytes AES key + 32 bytes HMAC key) using PBKDF2
    std::vector<uint8_t> derivedKeys(64);
    Crypto::PBKDF2::deriveKey(
        m_passphrase,
        salt.data(), salt.size(),
        m_iterations,
        derivedKeys.data(), derivedKeys.size()
    );

    const uint8_t* aesKey = derivedKeys.data();
    const uint8_t* hmacKey = derivedKeys.data() + 32;

    // 4. Encrypt using AES-256-CBC
    Crypto::AES256 cipher(aesKey);
    std::vector<uint8_t> ciphertext = cipher.encryptCBC(plaintext.data(), plaintext.size(), iv.data());

    // Wipe plaintext immediately
    Crypto::secureZeroContainer(plaintext);

    // 5. Construct Vault Header
    VaultHeader header = VaultFormat::createHeader(
        m_iterations,
        salt.data(),
        iv.data(),
        ciphertext.size()
    );

    // 6. Compute HMAC-SHA256 over Header (pre-HMAC bytes) + Ciphertext
    auto headerBytes = VaultFormat::getHeaderBytesForMAC(header);
    Crypto::HMAC_SHA256 mac(hmacKey, 32);
    mac.update(headerBytes);
    mac.update(ciphertext);
    auto tag = mac.finalize();
    std::memcpy(header.hmac, tag.data(), HMAC_SIZE);

    // Wipe sensitive derived keys
    Crypto::secureZeroContainer(derivedKeys);

    // 7. Write to output file (.vault)
    std::filesystem::path outPath(m_vaultPath);
    if (outPath.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(outPath.parent_path(), ec);
    }

    std::ofstream out(m_vaultPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Failed to open vault file for writing: " + m_vaultPath);
    }

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(ciphertext.data()), static_cast<std::streamsize>(ciphertext.size()));
    out.close();

    if (!out.good()) {
        throw std::runtime_error("Write error encountered while saving vault: " + m_vaultPath);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    PackStats stats;
    stats.fileCount = fileItems.size();
    stats.totalOriginalBytes = totalOriginalBytes;
    stats.totalEncryptedBytes = ciphertext.size();
    stats.vaultFileSizeBytes = sizeof(header) + ciphertext.size();
    stats.durationMs = duration;

    return stats;
}

} // namespace SecureVault

