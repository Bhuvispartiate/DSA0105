#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace SecureVault {

constexpr char VAULT_MAGIC[6] = {'S', 'V', 'A', 'U', 'L', 'T'};
constexpr uint16_t VAULT_VERSION_1 = 1;

enum class CipherAlgorithm : uint16_t {
    AES_256_CBC = 1
};

enum class KdfAlgorithm : uint16_t {
    PBKDF2_HMAC_SHA256 = 1
};

constexpr uint32_t DEFAULT_PBKDF2_ITERATIONS = 100000;
constexpr size_t SALT_SIZE = 32;
constexpr size_t IV_SIZE = 16;
constexpr size_t HMAC_SIZE = 32;

#pragma pack(push, 1)
struct VaultHeader {
    char magic[6];                // "SVAULT"
    uint16_t version;             // 1
    uint16_t cipherId;            // CipherAlgorithm
    uint16_t kdfId;               // KdfAlgorithm
    uint32_t kdfIterations;       // e.g. 100,000
    uint8_t salt[SALT_SIZE];      // 32-byte CSPRNG salt
    uint8_t iv[IV_SIZE];          // 16-byte CSPRNG IV
    uint64_t ciphertextLength;    // Length of encrypted payload
    uint8_t hmac[HMAC_SIZE];      // HMAC-SHA256 authentication tag
};
#pragma pack(pop)

static_assert(sizeof(VaultHeader) == 104, "VaultHeader must be exactly 104 bytes");

class VaultFormat {
public:
    static VaultHeader createHeader(
        uint32_t iterations,
        const uint8_t salt[SALT_SIZE],
        const uint8_t iv[IV_SIZE],
        uint64_t ciphertextLen
    );

    static bool validateHeader(const VaultHeader& header);

    // Serializes header fields excluding the HMAC tag for MAC computation
    static std::vector<uint8_t> getHeaderBytesForMAC(const VaultHeader& header);
};

} // namespace SecureVault

