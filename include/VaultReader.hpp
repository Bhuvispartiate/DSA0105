#pragma once

#include <string>
#include <vector>
#include <functional>
#include "VaultEntry.hpp"
#include "VaultFormat.hpp"

namespace SecureVault {

struct UnpackStats {
    size_t fileCount;
    uint64_t totalExtractedBytes;
    double durationMs;
};

class VaultReader {
public:
    using ProgressCallback = std::function<void(const std::string& currentFile, size_t fileIndex, size_t totalFiles)>;

    explicit VaultReader(const std::string& vaultPath);
    ~VaultReader();

    // Inspect header information without requiring password
    const VaultHeader& getHeader() const;

    // Verifies passphrase against vault HMAC tag without writing files to disk
    bool verifyPassphrase(const std::string& passphrase);

    // Unlocks and loads vault catalog into memory
    bool unlock(const std::string& passphrase);

    // Returns true if vault has been successfully unlocked
    bool isUnlocked() const;

    // Lists entries stored in the vault (requires unlock first)
    const std::vector<VaultEntry>& getEntries() const;

    // Extracts all files to target directory
    UnpackStats extractAll(const std::string& targetDirectory, ProgressCallback callback = nullptr);

    // Extracts a specific file by relative path
    bool extractFile(const std::string& relativePath, const std::string& targetDirectory);

private:
    std::string m_vaultPath;
    VaultHeader m_header;
    bool m_headerLoaded;
    bool m_unlocked;
    std::vector<VaultEntry> m_entries;
    std::vector<uint8_t> m_ciphertext;

    void loadHeaderAndCiphertext();
    bool deserializePayload(const std::vector<uint8_t>& plaintext);
};

} // namespace SecureVault

