#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <array>
#include <memory>

namespace SecureVault {
namespace Crypto {

// Securely wipes memory containing keys or sensitive plaintext
void secureZero(void* ptr, size_t size);

template <typename T>
void secureZeroContainer(T& container) {
    if (!container.empty()) {
        secureZero(container.data(), container.size() * sizeof(typename T::value_type));
    }
}

// Cryptographically secure pseudorandom byte generator
void getRandomBytes(uint8_t* buffer, size_t length);
std::vector<uint8_t> getRandomBytes(size_t length);

// SHA-256 digest computation
class SHA256 {
public:
    static constexpr size_t DIGEST_SIZE = 32;
    static constexpr size_t BLOCK_SIZE = 64;

    SHA256();
    void update(const uint8_t* data, size_t length);
    void update(const std::vector<uint8_t>& data);
    void update(const std::string& str);
    std::array<uint8_t, DIGEST_SIZE> finalize();

    static std::array<uint8_t, DIGEST_SIZE> hash(const uint8_t* data, size_t length);
    static std::array<uint8_t, DIGEST_SIZE> hash(const std::vector<uint8_t>& data);
    static std::string toHexString(const uint8_t* digest, size_t length = DIGEST_SIZE);

private:
    uint32_t m_state[8];
    uint64_t m_bitCount;
    uint8_t m_buffer[BLOCK_SIZE];

    void transform(const uint8_t* chunk);
};

// HMAC-SHA256 message authentication code
class HMAC_SHA256 {
public:
    static constexpr size_t MAC_SIZE = 32;

    HMAC_SHA256(const uint8_t* key, size_t keyLength);
    HMAC_SHA256(const std::vector<uint8_t>& key);
    ~HMAC_SHA256();

    void update(const uint8_t* data, size_t length);
    void update(const std::vector<uint8_t>& data);
    void update(const std::string& str);
    std::array<uint8_t, MAC_SIZE> finalize();

    static std::array<uint8_t, MAC_SIZE> compute(
        const uint8_t* key, size_t keyLength,
        const uint8_t* data, size_t dataLength
    );

    // Constant-time comparison to prevent timing attacks
    static bool verify(const uint8_t* a, const uint8_t* b, size_t length = MAC_SIZE);

private:
    SHA256 m_innerHasher;
    std::array<uint8_t, SHA256::BLOCK_SIZE> m_outerKeyPad;
};

// PBKDF2 key derivation using HMAC-SHA256 (RFC 2898)
class PBKDF2 {
public:
    static void deriveKey(
        const std::string& passphrase,
        const uint8_t* salt, size_t saltLength,
        uint32_t iterations,
        uint8_t* outputKey, size_t outputKeyLength
    );
};

// AES-256 cipher (CBC mode with PKCS#7 padding)
class AES256 {
public:
    static constexpr size_t KEY_SIZE = 32;   // 256-bit key
    static constexpr size_t BLOCK_SIZE = 16; // 128-bit block
    static constexpr size_t IV_SIZE = 16;    // 128-bit IV

    AES256(const uint8_t* key);
    ~AES256();

    // Raw block operations
    void encryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const;
    void decryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const;

    // CBC mode with PKCS#7 padding
    std::vector<uint8_t> encryptCBC(
        const uint8_t* plaintext, size_t length,
        const uint8_t iv[IV_SIZE]
    ) const;

    bool decryptCBC(
        const uint8_t* ciphertext, size_t length,
        const uint8_t iv[IV_SIZE],
        std::vector<uint8_t>& outPlaintext
    ) const;

private:
    uint32_t m_roundKeysEnc[60]; // 14 rounds + 1
    uint32_t m_roundKeysDec[60];

    void keyExpansion(const uint8_t* key);
};

} // namespace Crypto
} // namespace SecureVault

