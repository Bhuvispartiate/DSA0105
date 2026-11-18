#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <chrono>

namespace SecureVault {

struct VaultEntry {
    std::string relativePath;
    uint64_t fileSize;
    uint64_t modifiedTime; // Unix timestamp in seconds
    std::array<uint8_t, 32> sha256;
    std::vector<uint8_t> data; // populated in memory or during extraction

    VaultEntry() : fileSize(0), modifiedTime(0), sha256{} {}

    VaultEntry(
        const std::string& path,
        uint64_t size,
        uint64_t mtime,
        const std::array<uint8_t, 32>& hash
    ) : relativePath(path), fileSize(size), modifiedTime(mtime), sha256(hash) {}
};

} // namespace SecureVault

