#include "VaultReader.hpp"
#include "VaultFormat.hpp"
#include "Crypto.hpp"
#include "FileUtils.hpp"

#include <fstream>
#include <stdexcept>
#include <chrono>
#include <cstring>

namespace SecureVault {

VaultReader::VaultReader(const std::string& vaultPath)
    : m_vaultPath(vaultPath),
      m_header{},
      m_headerLoaded(false),
      m_unlocked(false) {
    loadHeaderAndCiphertext();
}

VaultReader::~VaultReader() {
    for (auto& entry : m_entries) {
        Crypto::secureZeroContainer(entry.data);
    }
}

void VaultReader::loadHeaderAndCiphertext() {
    if (m_headerLoaded) return;

    std::ifstream file(m_vaultPath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Could not open vault file: " + m_vaultPath);
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize < static_cast<std::streamsize>(sizeof(VaultHeader))) {
        throw std::runtime_error("Invalid vault file: file size is smaller than header specification.");
    }

    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&m_header), sizeof(VaultHeader))) {
        throw std::runtime_error("Failed to read header from vault file: " + m_vaultPath);
    }

    if (!VaultFormat::validateHeader(m_header)) {
        throw std::runtime_error("Corrupted or unsupported vault header format in: " + m_vaultPath);
    }

    uint64_t expectedRemaining = static_cast<uint64_t>(fileSize) - sizeof(VaultHeader);
    if (m_header.ciphertextLength != expectedRemaining) {
        throw std::runtime_error("Vault payload size mismatch: header reports " +
                                 std::to_string(m_header.ciphertextLength) + " bytes, but found " +
                                 std::to_string(expectedRemaining) + " bytes.");
    }

    m_ciphertext.resize(static_cast<size_t>(m_header.ciphertextLength));
    if (m_header.ciphertextLength > 0) {
        if (!file.read(reinterpret_cast<char*>(m_ciphertext.data()), static_cast<std::streamsize>(m_header.ciphertextLength))) {
            throw std::runtime_error("Failed to read encrypted payload from vault file: " + m_vaultPath);
        }
    }

    m_headerLoaded = true;
}

const VaultHeader& VaultReader::getHeader() const {
    return m_header;
}

bool VaultReader::verifyPassphrase(const std::string& passphrase) {
    if (passphrase.empty()) return false;
    if (!m_headerLoaded) return false;

    // Derive 64 bytes master key (AES 32 bytes + HMAC 32 bytes)
    std::vector<uint8_t> derivedKeys(64);
    Crypto::PBKDF2::deriveKey(
        passphrase,
        m_header.salt, SALT_SIZE,
        m_header.kdfIterations,
        derivedKeys.data(), derivedKeys.size()
    );

    const uint8_t* hmacKey = derivedKeys.data() + 32;

    // Compute HMAC-SHA256
    auto headerBytes = VaultFormat::getHeaderBytesForMAC(m_header);
    Crypto::HMAC_SHA256 mac(hmacKey, 32);
    mac.update(headerBytes);
    mac.update(m_ciphertext);
    auto calculatedTag = mac.finalize();

    // Constant-time tag verification
    bool match = Crypto::HMAC_SHA256::verify(calculatedTag.data(), m_header.hmac, HMAC_SIZE);

    Crypto::secureZeroContainer(derivedKeys);
    return match;
}

bool VaultReader::unlock(const std::string& passphrase) {
    if (m_unlocked) return true;
    if (passphrase.empty()) return false;

    // Derive keys
    std::vector<uint8_t> derivedKeys(64);
    Crypto::PBKDF2::deriveKey(
        passphrase,
        m_header.salt, SALT_SIZE,
        m_header.kdfIterations,
        derivedKeys.data(), derivedKeys.size()
    );

    const uint8_t* aesKey = derivedKeys.data();
    const uint8_t* hmacKey = derivedKeys.data() + 32;

    // 1. Verify HMAC first (Encrypt-then-MAC prevents padding oracle attacks)
    auto headerBytes = VaultFormat::getHeaderBytesForMAC(m_header);
    Crypto::HMAC_SHA256 mac(hmacKey, 32);
    mac.update(headerBytes);
    mac.update(m_ciphertext);
    auto calculatedTag = mac.finalize();

    if (!Crypto::HMAC_SHA256::verify(calculatedTag.data(), m_header.hmac, HMAC_SIZE)) {
        Crypto::secureZeroContainer(derivedKeys);
        return false; // Authentication failed (wrong key or altered data)
    }

    // 2. Decrypt ciphertext using AES-256-CBC
    Crypto::AES256 cipher(aesKey);
    std::vector<uint8_t> plaintext;
    bool decSuccess = cipher.decryptCBC(m_ciphertext.data(), m_ciphertext.size(), m_header.iv, plaintext);

    Crypto::secureZeroContainer(derivedKeys);

    if (!decSuccess) {
        return false;
    }

    // 3. Deserialize plaintext into entry objects
    bool parsed = deserializePayload(plaintext);
    Crypto::secureZeroContainer(plaintext);

    if (!parsed) {
        return false;
    }

    m_unlocked = true;
    return true;
}

bool VaultReader::isUnlocked() const {
    return m_unlocked;
}

const std::vector<VaultEntry>& VaultReader::getEntries() const {
    if (!m_unlocked) {
        throw std::runtime_error("Vault must be unlocked with the correct key before accessing entries.");
    }
    return m_entries;
}

bool VaultReader::deserializePayload(const std::vector<uint8_t>& plaintext) {
    if (plaintext.size() < sizeof(uint32_t)) {
        return false;
    }

    size_t offset = 0;

    auto readUint32 = [&](uint32_t& outVal) -> bool {
        if (offset + 4 > plaintext.size()) return false;
        outVal = (static_cast<uint32_t>(plaintext[offset + 0]) << 24) |
                 (static_cast<uint32_t>(plaintext[offset + 1]) << 16) |
                 (static_cast<uint32_t>(plaintext[offset + 2]) << 8)  |
                 (static_cast<uint32_t>(plaintext[offset + 3]));
        offset += 4;
        return true;
    };

    auto readUint64 = [&](uint64_t& outVal) -> bool {
        if (offset + 8 > plaintext.size()) return false;
        outVal = (static_cast<uint64_t>(plaintext[offset + 0]) << 56) |
                 (static_cast<uint64_t>(plaintext[offset + 1]) << 48) |
                 (static_cast<uint64_t>(plaintext[offset + 2]) << 40) |
                 (static_cast<uint64_t>(plaintext[offset + 3]) << 32) |
                 (static_cast<uint64_t>(plaintext[offset + 4]) << 24) |
                 (static_cast<uint64_t>(plaintext[offset + 5]) << 16) |
                 (static_cast<uint64_t>(plaintext[offset + 6]) << 8)  |
                 (static_cast<uint64_t>(plaintext[offset + 7]));
        offset += 8;
        return true;
    };

    uint32_t entryCount = 0;
    if (!readUint32(entryCount)) return false;

    m_entries.clear();
    m_entries.reserve(entryCount);

    for (uint32_t i = 0; i < entryCount; ++i) {
        uint32_t pathLen = 0;
        if (!readUint32(pathLen)) return false;
        if (offset + pathLen > plaintext.size()) return false;

        std::string relPath(reinterpret_cast<const char*>(plaintext.data() + offset), pathLen);
        offset += pathLen;

        uint64_t fileSize = 0;
        if (!readUint64(fileSize)) return false;

        uint64_t mtime = 0;
        if (!readUint64(mtime)) return false;

        if (offset + 32 > plaintext.size()) return false;
        std::array<uint8_t, 32> expectedHash{};
        std::memcpy(expectedHash.data(), plaintext.data() + offset, 32);
        offset += 32;

        if (offset + fileSize > plaintext.size()) return false;

        VaultEntry entry;
        entry.relativePath = relPath;
        entry.fileSize = fileSize;
        entry.modifiedTime = mtime;
        entry.sha256 = expectedHash;
        entry.data.assign(plaintext.data() + offset, plaintext.data() + offset + fileSize);
        offset += static_cast<size_t>(fileSize);

        // Verify payload SHA-256 checksum
        auto actualHash = Crypto::SHA256::hash(entry.data);
        if (std::memcmp(actualHash.data(), expectedHash.data(), 32) != 0) {
            return false; // File-level corruption detected
        }

        m_entries.push_back(std::move(entry));
    }

    return true;
}

UnpackStats VaultReader::extractAll(const std::string& targetDirectory, ProgressCallback callback) {
    if (!m_unlocked) {
        throw std::runtime_error("Cannot extract files: Vault is locked.");
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    std::filesystem::path targetBase(targetDirectory);

    uint64_t totalBytes = 0;
    size_t count = 0;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& entry = m_entries[i];

        if (!FileUtils::isSafeRelativePath(entry.relativePath)) {
            continue; // Skip dangerous paths
        }

        if (callback) {
            callback(entry.relativePath, i + 1, m_entries.size());
        }

        std::filesystem::path dest = targetBase / entry.relativePath;
        if (dest.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(dest.parent_path(), ec);
        }

        if (!FileUtils::writeFile(dest, entry.data)) {
            throw std::runtime_error("Failed to write extracted file to: " + dest.string());
        }

        totalBytes += entry.fileSize;
        count++;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    UnpackStats stats;
    stats.fileCount = count;
    stats.totalExtractedBytes = totalBytes;
    stats.durationMs = duration;
    return stats;
}

bool VaultReader::extractFile(const std::string& relativePath, const std::string& targetDirectory) {
    if (!m_unlocked) {
        throw std::runtime_error("Cannot extract file: Vault is locked.");
    }

    for (const auto& entry : m_entries) {
        if (entry.relativePath == relativePath) {
            if (!FileUtils::isSafeRelativePath(entry.relativePath)) {
                return false;
            }
            std::filesystem::path dest = std::filesystem::path(targetDirectory) / entry.relativePath;
            return FileUtils::writeFile(dest, entry.data);
        }
    }
    return false;
}

} // namespace SecureVault

