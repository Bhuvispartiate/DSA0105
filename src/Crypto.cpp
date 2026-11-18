#include "Crypto.hpp"

#include <cstring>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace SecureVault {
namespace Crypto {

// Memory wipe
void secureZero(void* ptr, size_t size) {
    if (!ptr || size == 0) return;
#if defined(_WIN32)
    SecureZeroMemory(ptr, size);
#else
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    while (size--) {
        *p++ = 0;
    }
#endif
}

// CSPRNG implementation
void getRandomBytes(uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
#if defined(_WIN32)
    HCRYPTPROV hProvider = 0;
    if (CryptAcquireContextW(&hProvider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        if (CryptGenRandom(hProvider, static_cast<DWORD>(length), buffer)) {
            CryptReleaseContext(hProvider, 0);
            return;
        }
        CryptReleaseContext(hProvider, 0);
    }
    throw std::runtime_error("CryptGenRandom failed to acquire cryptographic randomness.");
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        size_t total = 0;
        while (total < length) {
            ssize_t bytesRead = read(fd, buffer + total, length - total);
            if (bytesRead <= 0) break;
            total += static_cast<size_t>(bytesRead);
        }
        close(fd);
        if (total == length) return;
    }
    throw std::runtime_error("Failed to read from /dev/urandom.");
#endif
}

std::vector<uint8_t> getRandomBytes(size_t length) {
    std::vector<uint8_t> bytes(length);
    getRandomBytes(bytes.data(), length);
    return bytes;
}

// ============================================================================
// SHA-256 (FIPS 180-4) Implementation
// ============================================================================

namespace {
    inline uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }

    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    inline uint32_t sigma0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    inline uint32_t sigma1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

    inline uint32_t gamma0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    inline uint32_t gamma1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    const uint32_t K256[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
} // anonymous namespace

SHA256::SHA256() {
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
    m_bitCount = 0;
}

void SHA256::transform(const uint8_t* chunk) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(chunk[i * 4 + 0]) << 24) |
               (static_cast<uint32_t>(chunk[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(chunk[i * 4 + 2]) << 8)  |
               (static_cast<uint32_t>(chunk[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = m_state[0];
    uint32_t b = m_state[1];
    uint32_t c = m_state[2];
    uint32_t d = m_state[3];
    uint32_t e = m_state[4];
    uint32_t f = m_state[5];
    uint32_t g = m_state[6];
    uint32_t h = m_state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K256[i] + w[i];
        uint32_t t2 = sigma0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;
}

void SHA256::update(const uint8_t* data, size_t length) {
    if (!data || length == 0) return;
    size_t bufferIndex = static_cast<size_t>((m_bitCount / 8) % BLOCK_SIZE);
    m_bitCount += static_cast<uint64_t>(length) * 8;

    size_t spaceInBuf = BLOCK_SIZE - bufferIndex;
    if (length >= spaceInBuf) {
        std::memcpy(m_buffer + bufferIndex, data, spaceInBuf);
        transform(m_buffer);
        data += spaceInBuf;
        length -= spaceInBuf;
        while (length >= BLOCK_SIZE) {
            transform(data);
            data += BLOCK_SIZE;
            length -= BLOCK_SIZE;
        }
        bufferIndex = 0;
    }

    if (length > 0) {
        std::memcpy(m_buffer + bufferIndex, data, length);
    }
}

void SHA256::update(const std::vector<uint8_t>& data) {
    update(data.data(), data.size());
}

void SHA256::update(const std::string& str) {
    update(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

std::array<uint8_t, SHA256::DIGEST_SIZE> SHA256::finalize() {
    uint64_t totalBits = m_bitCount;
    size_t bufferIndex = static_cast<size_t>((m_bitCount / 8) % BLOCK_SIZE);

    m_buffer[bufferIndex++] = 0x80;
    if (bufferIndex > 56) {
        std::memset(m_buffer + bufferIndex, 0, BLOCK_SIZE - bufferIndex);
        transform(m_buffer);
        bufferIndex = 0;
    }
    std::memset(m_buffer + bufferIndex, 0, 56 - bufferIndex);

    // Append 64-bit length in big-endian bits
    for (int i = 0; i < 8; ++i) {
        m_buffer[63 - i] = static_cast<uint8_t>(totalBits >> (i * 8));
    }
    transform(m_buffer);

    std::array<uint8_t, DIGEST_SIZE> digest;
    for (int i = 0; i < 8; ++i) {
        digest[i * 4 + 0] = static_cast<uint8_t>(m_state[i] >> 24);
        digest[i * 4 + 1] = static_cast<uint8_t>(m_state[i] >> 16);
        digest[i * 4 + 2] = static_cast<uint8_t>(m_state[i] >> 8);
        digest[i * 4 + 3] = static_cast<uint8_t>(m_state[i]);
    }
    secureZero(m_state, sizeof(m_state));
    secureZero(m_buffer, sizeof(m_buffer));
    return digest;
}

std::array<uint8_t, SHA256::DIGEST_SIZE> SHA256::hash(const uint8_t* data, size_t length) {
    SHA256 hasher;
    hasher.update(data, length);
    return hasher.finalize();
}

std::array<uint8_t, SHA256::DIGEST_SIZE> SHA256::hash(const std::vector<uint8_t>& data) {
    return hash(data.data(), data.size());
}

std::string SHA256::toHexString(const uint8_t* digest, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return oss.str();
}

// ============================================================================
// HMAC-SHA256 Implementation (RFC 2104)
// ============================================================================

HMAC_SHA256::HMAC_SHA256(const uint8_t* key, size_t keyLength) {
    std::array<uint8_t, SHA256::BLOCK_SIZE> keyBlock{};
    if (keyLength > SHA256::BLOCK_SIZE) {
        auto keyHash = SHA256::hash(key, keyLength);
        std::memcpy(keyBlock.data(), keyHash.data(), SHA256::DIGEST_SIZE);
    } else {
        std::memcpy(keyBlock.data(), key, keyLength);
    }

    std::array<uint8_t, SHA256::BLOCK_SIZE> innerKeyPad{};
    for (size_t i = 0; i < SHA256::BLOCK_SIZE; ++i) {
        innerKeyPad[i] = keyBlock[i] ^ 0x36;
        m_outerKeyPad[i] = keyBlock[i] ^ 0x5c;
    }

    m_innerHasher.update(innerKeyPad.data(), innerKeyPad.size());
    secureZero(keyBlock.data(), keyBlock.size());
    secureZero(innerKeyPad.data(), innerKeyPad.size());
}

HMAC_SHA256::HMAC_SHA256(const std::vector<uint8_t>& key)
    : HMAC_SHA256(key.data(), key.size()) {}

HMAC_SHA256::~HMAC_SHA256() {
    secureZero(m_outerKeyPad.data(), m_outerKeyPad.size());
}

void HMAC_SHA256::update(const uint8_t* data, size_t length) {
    m_innerHasher.update(data, length);
}

void HMAC_SHA256::update(const std::vector<uint8_t>& data) {
    m_innerHasher.update(data.data(), data.size());
}

void HMAC_SHA256::update(const std::string& str) {
    m_innerHasher.update(str);
}

std::array<uint8_t, HMAC_SHA256::MAC_SIZE> HMAC_SHA256::finalize() {
    auto innerHash = m_innerHasher.finalize();
    SHA256 outerHasher;
    outerHasher.update(m_outerKeyPad.data(), m_outerKeyPad.size());
    outerHasher.update(innerHash.data(), innerHash.size());
    secureZero(innerHash.data(), innerHash.size());
    return outerHasher.finalize();
}

std::array<uint8_t, HMAC_SHA256::MAC_SIZE> HMAC_SHA256::compute(
    const uint8_t* key, size_t keyLength,
    const uint8_t* data, size_t dataLength
) {
    HMAC_SHA256 hmac(key, keyLength);
    hmac.update(data, dataLength);
    return hmac.finalize();
}

bool HMAC_SHA256::verify(const uint8_t* a, const uint8_t* b, size_t length) {
    uint8_t diff = 0;
    for (size_t i = 0; i < length; ++i) {
        diff |= (a[i] ^ b[i]);
    }
    return diff == 0;
}

// ============================================================================
// PBKDF2-HMAC-SHA256 (RFC 8018)
// ============================================================================

void PBKDF2::deriveKey(
    const std::string& passphrase,
    const uint8_t* salt, size_t saltLength,
    uint32_t iterations,
    uint8_t* outputKey, size_t outputKeyLength
) {
    const uint8_t* passBytes = reinterpret_cast<const uint8_t*>(passphrase.data());
    size_t passLen = passphrase.size();

    size_t hLen = SHA256::DIGEST_SIZE;
    uint32_t totalBlocks = static_cast<uint32_t>((outputKeyLength + hLen - 1) / hLen);

    for (uint32_t i = 1; i <= totalBlocks; ++i) {
        // U_1 = PRF(P, S || INT_32_BE(i))
        HMAC_SHA256 hmac(passBytes, passLen);
        hmac.update(salt, saltLength);
        uint8_t blockIndexBytes[4] = {
            static_cast<uint8_t>(i >> 24),
            static_cast<uint8_t>(i >> 16),
            static_cast<uint8_t>(i >> 8),
            static_cast<uint8_t>(i)
        };
        hmac.update(blockIndexBytes, 4);
        auto u = hmac.finalize();
        auto f = u;

        // U_2 ... U_c
        for (uint32_t iter = 1; iter < iterations; ++iter) {
            HMAC_SHA256 iterHmac(passBytes, passLen);
            iterHmac.update(u.data(), u.size());
            u = iterHmac.finalize();
            for (size_t k = 0; k < hLen; ++k) {
                f[k] ^= u[k];
            }
        }

        size_t offset = (i - 1) * hLen;
        size_t toCopy = std::min(hLen, outputKeyLength - offset);
        std::memcpy(outputKey + offset, f.data(), toCopy);

        secureZero(u.data(), u.size());
        secureZero(f.data(), f.size());
    }
}

// ============================================================================
// AES-256 (FIPS 197) Implementation
// ============================================================================

namespace {
    const uint8_t sbox[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    const uint8_t rsbox[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
    };

    const uint32_t rcon[15] = {
        0x00000000, 0x01000000, 0x02000000, 0x04000000, 0x08000000,
        0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1b000000,
        0x36000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    };

    inline uint32_t subWord(uint32_t w) {
        return (static_cast<uint32_t>(sbox[(w >> 24) & 0xff]) << 24) |
               (static_cast<uint32_t>(sbox[(w >> 16) & 0xff]) << 16) |
               (static_cast<uint32_t>(sbox[(w >> 8)  & 0xff]) << 8)  |
               (static_cast<uint32_t>(sbox[w & 0xff]));
    }

    inline uint32_t rotWord(uint32_t w) {
        return (w << 8) | (w >> 24);
    }

    inline uint8_t xtime(uint8_t x) {
        return static_cast<uint8_t>((x << 1) ^ (((x >> 7) & 1) * 0x1b));
    }

    inline uint8_t multiplyGF(uint8_t a, uint8_t b) {
        uint8_t p = 0;
        for (int i = 0; i < 8; ++i) {
            if (b & 1) p ^= a;
            a = xtime(a);
            b >>= 1;
        }
        return p;
    }
} // anonymous namespace

AES256::AES256(const uint8_t* key) {
    keyExpansion(key);
}

AES256::~AES256() {
    secureZero(m_roundKeysEnc, sizeof(m_roundKeysEnc));
    secureZero(m_roundKeysDec, sizeof(m_roundKeysDec));
}

void AES256::keyExpansion(const uint8_t* key) {
    // 256-bit key: Nk = 8, Nr = 14. Total round words = 4 * (14 + 1) = 60
    for (int i = 0; i < 8; ++i) {
        m_roundKeysEnc[i] = (static_cast<uint32_t>(key[i * 4 + 0]) << 24) |
                            (static_cast<uint32_t>(key[i * 4 + 1]) << 16) |
                            (static_cast<uint32_t>(key[i * 4 + 2]) << 8)  |
                            (static_cast<uint32_t>(key[i * 4 + 3]));
    }

    for (int i = 8; i < 60; ++i) {
        uint32_t temp = m_roundKeysEnc[i - 1];
        if (i % 8 == 0) {
            temp = subWord(rotWord(temp)) ^ rcon[i / 8];
        } else if (i % 8 == 4) {
            temp = subWord(temp);
        }
        m_roundKeysEnc[i] = m_roundKeysEnc[i - 8] ^ temp;
    }

    // Prepare decryption round keys (Equivalent Inverse Cipher)
    for (int r = 0; r <= 14; ++r) {
        int encRound = 14 - r;
        for (int c = 0; c < 4; ++c) {
            uint32_t w = m_roundKeysEnc[encRound * 4 + c];
            if (r == 0 || r == 14) {
                m_roundKeysDec[r * 4 + c] = w;
            } else {
                // Apply InvMixColumns to the round key words
                uint8_t b0 = static_cast<uint8_t>(w >> 24);
                uint8_t b1 = static_cast<uint8_t>(w >> 16);
                uint8_t b2 = static_cast<uint8_t>(w >> 8);
                uint8_t b3 = static_cast<uint8_t>(w);

                uint8_t d0 = multiplyGF(b0, 0x0e) ^ multiplyGF(b1, 0x0b) ^ multiplyGF(b2, 0x0d) ^ multiplyGF(b3, 0x09);
                uint8_t d1 = multiplyGF(b0, 0x09) ^ multiplyGF(b1, 0x0e) ^ multiplyGF(b2, 0x0b) ^ multiplyGF(b3, 0x0d);
                uint8_t d2 = multiplyGF(b0, 0x0d) ^ multiplyGF(b1, 0x09) ^ multiplyGF(b2, 0x0e) ^ multiplyGF(b3, 0x0b);
                uint8_t d3 = multiplyGF(b0, 0x0b) ^ multiplyGF(b1, 0x0d) ^ multiplyGF(b2, 0x09) ^ multiplyGF(b3, 0x0e);

                m_roundKeysDec[r * 4 + c] = (static_cast<uint32_t>(d0) << 24) |
                                            (static_cast<uint32_t>(d1) << 16) |
                                            (static_cast<uint32_t>(d2) << 8)  |
                                            (static_cast<uint32_t>(d3));
            }
        }
    }
}

void AES256::encryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const {
    uint8_t state[4][4];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = in[r + 4 * c];
        }
    }

    // Initial round: AddRoundKey 0
    for (int c = 0; c < 4; ++c) {
        uint32_t rk = m_roundKeysEnc[c];
        state[0][c] ^= static_cast<uint8_t>(rk >> 24);
        state[1][c] ^= static_cast<uint8_t>(rk >> 16);
        state[2][c] ^= static_cast<uint8_t>(rk >> 8);
        state[3][c] ^= static_cast<uint8_t>(rk);
    }

    // Rounds 1 to 13
    for (int round = 1; round < 14; ++round) {
        // SubBytes
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                state[r][c] = sbox[state[r][c]];
            }
        }
        // ShiftRows
        uint8_t temp = state[1][0];
        state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = temp;
        std::swap(state[2][0], state[2][2]);
        std::swap(state[2][1], state[2][3]);
        temp = state[3][3];
        state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = state[3][0]; state[3][0] = temp;

        // MixColumns
        for (int c = 0; c < 4; ++c) {
            uint8_t a0 = state[0][c];
            uint8_t a1 = state[1][c];
            uint8_t a2 = state[2][c];
            uint8_t a3 = state[3][c];
            state[0][c] = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
            state[1][c] = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
            state[2][c] = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
            state[3][c] = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);
        }

        // AddRoundKey
        for (int c = 0; c < 4; ++c) {
            uint32_t rk = m_roundKeysEnc[round * 4 + c];
            state[0][c] ^= static_cast<uint8_t>(rk >> 24);
            state[1][c] ^= static_cast<uint8_t>(rk >> 16);
            state[2][c] ^= static_cast<uint8_t>(rk >> 8);
            state[3][c] ^= static_cast<uint8_t>(rk);
        }
    }

    // Final Round 14: SubBytes, ShiftRows, AddRoundKey (No MixColumns)
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = sbox[state[r][c]];
        }
    }
    uint8_t temp = state[1][0];
    state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = temp;
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);
    temp = state[3][3];
    state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = state[3][0]; state[3][0] = temp;

    for (int c = 0; c < 4; ++c) {
        uint32_t rk = m_roundKeysEnc[14 * 4 + c];
        state[0][c] ^= static_cast<uint8_t>(rk >> 24);
        state[1][c] ^= static_cast<uint8_t>(rk >> 16);
        state[2][c] ^= static_cast<uint8_t>(rk >> 8);
        state[3][c] ^= static_cast<uint8_t>(rk);
    }

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out[r + 4 * c] = state[r][c];
        }
    }
}

void AES256::decryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const {
    uint8_t state[4][4];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = in[r + 4 * c];
        }
    }

    // Initial round: AddRoundKey 0 using m_roundKeysDec
    for (int c = 0; c < 4; ++c) {
        uint32_t rk = m_roundKeysDec[c];
        state[0][c] ^= static_cast<uint8_t>(rk >> 24);
        state[1][c] ^= static_cast<uint8_t>(rk >> 16);
        state[2][c] ^= static_cast<uint8_t>(rk >> 8);
        state[3][c] ^= static_cast<uint8_t>(rk);
    }

    // Rounds 1 to 13
    for (int round = 1; round < 14; ++round) {
        // InvSubBytes
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                state[r][c] = rsbox[state[r][c]];
            }
        }
        // InvShiftRows
        uint8_t t = state[1][3];
        state[1][3] = state[1][2]; state[1][2] = state[1][1]; state[1][1] = state[1][0]; state[1][0] = t;
        std::swap(state[2][0], state[2][2]);
        std::swap(state[2][1], state[2][3]);
        t = state[3][0];
        state[3][0] = state[3][1]; state[3][1] = state[3][2]; state[3][2] = state[3][3]; state[3][3] = t;

        // InvMixColumns
        for (int c = 0; c < 4; ++c) {
            uint8_t s0 = state[0][c];
            uint8_t s1 = state[1][c];
            uint8_t s2 = state[2][c];
            uint8_t s3 = state[3][c];
            state[0][c] = multiplyGF(s0, 0x0e) ^ multiplyGF(s1, 0x0b) ^ multiplyGF(s2, 0x0d) ^ multiplyGF(s3, 0x09);
            state[1][c] = multiplyGF(s0, 0x09) ^ multiplyGF(s1, 0x0e) ^ multiplyGF(s2, 0x0b) ^ multiplyGF(s3, 0x0d);
            state[2][c] = multiplyGF(s0, 0x0d) ^ multiplyGF(s1, 0x09) ^ multiplyGF(s2, 0x0e) ^ multiplyGF(s3, 0x0b);
            state[3][c] = multiplyGF(s0, 0x0b) ^ multiplyGF(s1, 0x0d) ^ multiplyGF(s2, 0x09) ^ multiplyGF(s3, 0x0e);
        }

        // AddRoundKey
        for (int c = 0; c < 4; ++c) {
            uint32_t rk = m_roundKeysDec[round * 4 + c];
            state[0][c] ^= static_cast<uint8_t>(rk >> 24);
            state[1][c] ^= static_cast<uint8_t>(rk >> 16);
            state[2][c] ^= static_cast<uint8_t>(rk >> 8);
            state[3][c] ^= static_cast<uint8_t>(rk);
        }
    }

    // Final Round 14: InvSubBytes, InvShiftRows, AddRoundKey (No InvMixColumns)
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = rsbox[state[r][c]];
        }
    }
    uint8_t t = state[1][3];
    state[1][3] = state[1][2]; state[1][2] = state[1][1]; state[1][1] = state[1][0]; state[1][0] = t;
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);
    t = state[3][0];
    state[3][0] = state[3][1]; state[3][1] = state[3][2]; state[3][2] = state[3][3]; state[3][3] = t;

    for (int c = 0; c < 4; ++c) {
        uint32_t rk = m_roundKeysDec[14 * 4 + c];
        state[0][c] ^= static_cast<uint8_t>(rk >> 24);
        state[1][c] ^= static_cast<uint8_t>(rk >> 16);
        state[2][c] ^= static_cast<uint8_t>(rk >> 8);
        state[3][c] ^= static_cast<uint8_t>(rk);
    }

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out[r + 4 * c] = state[r][c];
        }
    }
}

std::vector<uint8_t> AES256::encryptCBC(
    const uint8_t* plaintext, size_t length,
    const uint8_t iv[IV_SIZE]
) const {
    size_t padLen = BLOCK_SIZE - (length % BLOCK_SIZE);
    size_t totalLen = length + padLen;
    std::vector<uint8_t> ciphertext(totalLen);

    uint8_t previousBlock[BLOCK_SIZE];
    std::memcpy(previousBlock, iv, BLOCK_SIZE);

    size_t fullBlocks = length / BLOCK_SIZE;
    for (size_t b = 0; b < fullBlocks; ++b) {
        uint8_t block[BLOCK_SIZE];
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            block[i] = plaintext[b * BLOCK_SIZE + i] ^ previousBlock[i];
        }
        encryptBlock(block, ciphertext.data() + b * BLOCK_SIZE);
        std::memcpy(previousBlock, ciphertext.data() + b * BLOCK_SIZE, BLOCK_SIZE);
    }

    // Last block with PKCS#7 padding
    uint8_t lastBlock[BLOCK_SIZE];
    size_t remaining = length % BLOCK_SIZE;
    if (remaining > 0) {
        std::memcpy(lastBlock, plaintext + fullBlocks * BLOCK_SIZE, remaining);
    }
    for (size_t i = remaining; i < BLOCK_SIZE; ++i) {
        lastBlock[i] = static_cast<uint8_t>(padLen);
    }
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        lastBlock[i] ^= previousBlock[i];
    }
    encryptBlock(lastBlock, ciphertext.data() + fullBlocks * BLOCK_SIZE);

    secureZero(previousBlock, sizeof(previousBlock));
    secureZero(lastBlock, sizeof(lastBlock));
    return ciphertext;
}

bool AES256::decryptCBC(
    const uint8_t* ciphertext, size_t length,
    const uint8_t iv[IV_SIZE],
    std::vector<uint8_t>& outPlaintext
) const {
    if (length == 0 || (length % BLOCK_SIZE) != 0) {
        return false;
    }

    outPlaintext.resize(length);
    uint8_t previousBlock[BLOCK_SIZE];
    std::memcpy(previousBlock, iv, BLOCK_SIZE);

    size_t blocks = length / BLOCK_SIZE;
    for (size_t b = 0; b < blocks; ++b) {
        uint8_t rawDec[BLOCK_SIZE];
        decryptBlock(ciphertext + b * BLOCK_SIZE, rawDec);
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            outPlaintext[b * BLOCK_SIZE + i] = rawDec[i] ^ previousBlock[i];
        }
        std::memcpy(previousBlock, ciphertext + b * BLOCK_SIZE, BLOCK_SIZE);
    }

    // Validate PKCS#7 padding
    uint8_t padValue = outPlaintext.back();
    if (padValue == 0 || padValue > BLOCK_SIZE || padValue > length) {
        secureZeroContainer(outPlaintext);
        outPlaintext.clear();
        return false;
    }

    for (size_t i = length - padValue; i < length; ++i) {
        if (outPlaintext[i] != padValue) {
            secureZeroContainer(outPlaintext);
            outPlaintext.clear();
            return false;
        }
    }

    outPlaintext.resize(length - padValue);
    secureZero(previousBlock, sizeof(previousBlock));
    return true;
}

} // namespace Crypto
} // namespace SecureVault
