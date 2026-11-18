#include "VaultFormat.hpp"
#include <cstring>

namespace SecureVault {

VaultHeader VaultFormat::createHeader(
    uint32_t iterations,
    const uint8_t salt[SALT_SIZE],
    const uint8_t iv[IV_SIZE],
    uint64_t ciphertextLen
) {
    VaultHeader header{};
    std::memcpy(header.magic, VAULT_MAGIC, sizeof(VAULT_MAGIC));
    header.version = VAULT_VERSION_1;
    header.cipherId = static_cast<uint16_t>(CipherAlgorithm::AES_256_CBC);
    header.kdfId = static_cast<uint16_t>(KdfAlgorithm::PBKDF2_HMAC_SHA256);
    header.kdfIterations = iterations;
    std::memcpy(header.salt, salt, SALT_SIZE);
    std::memcpy(header.iv, iv, IV_SIZE);
    header.ciphertextLength = ciphertextLen;
    std::memset(header.hmac, 0, HMAC_SIZE);
    return header;
}

bool VaultFormat::validateHeader(const VaultHeader& header) {
    if (std::memcmp(header.magic, VAULT_MAGIC, sizeof(VAULT_MAGIC)) != 0) {
        return false;
    }
    if (header.version != VAULT_VERSION_1) {
        return false;
    }
    if (header.cipherId != static_cast<uint16_t>(CipherAlgorithm::AES_256_CBC)) {
        return false;
    }
    if (header.kdfId != static_cast<uint16_t>(KdfAlgorithm::PBKDF2_HMAC_SHA256)) {
        return false;
    }
    if (header.kdfIterations < 1000) {
        return false; // Reject dangerously low iterations
    }
    return true;
}

std::vector<uint8_t> VaultFormat::getHeaderBytesForMAC(const VaultHeader& header) {
    // Return all bytes preceding the hmac field
    size_t headerBytesToMac = sizeof(VaultHeader) - HMAC_SIZE;
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&header);
    return std::vector<uint8_t>(raw, raw + headerBytesToMac);
}

} // namespace SecureVault

