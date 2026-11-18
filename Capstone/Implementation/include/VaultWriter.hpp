#pragma once

#include <string>
#include <vector>
#include <functional>
#include "VaultEntry.hpp"
#include "VaultFormat.hpp"

namespace SecureVault {

struct PackStats {
    size_t fileCount;
    uint64_t totalOriginalBytes;
    uint64_t totalEncryptedBytes;
    uint64_t vaultFileSizeBytes;
    double durationMs;
};

class VaultWriter {
public:
    using ProgressCallback = std::function<void(const std::string& currentFile, size_t fileIndex, size_t totalFiles)>;

    VaultWriter(const std::string& vaultPath, const std::string& passphrase);
    ~VaultWriter();

    // Sets custom PBKDF2 iterations (default is 100,000)
    void setKdfIterations(uint32_t iterations);

    // Adds a file or directory path from disk
    void addSource(const std::string& path);

    // Adds a batch of sources
    void addSources(const std::vector<std::string>& paths);

    // Executes packing and encryption
    PackStats pack(ProgressCallback callback = nullptr);

private:
    std::string m_vaultPath;
    std::string m_passphrase;
    uint32_t m_iterations;
    std::vector<std::string> m_sources;

    std::vector<uint8_t> serializePayload(const std::vector<VaultEntry>& entries);
};

} // namespace SecureVault

