#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace SecureVault {
namespace FileUtils {

// Resolves and collects all files from a list of paths (directories are traversed recursively)
// Returns pairs of (absolute/relative input path on disk, relative path to store in vault)
struct FileItem {
    std::filesystem::path diskPath;
    std::string vaultRelativePath;
    uint64_t fileSize;
    uint64_t modifiedTime;
};

std::vector<FileItem> collectFiles(const std::vector<std::string>& inputPaths);

// Path sanitization to prevent ZipSlip / directory traversal vulnerabilities
bool isSafeRelativePath(const std::string& path);
std::string sanitizePath(const std::string& path);

// Reads an entire file into a byte vector
std::vector<uint8_t> readFile(const std::filesystem::path& path);

// Writes a byte buffer to a file, creating parent directories if needed
bool writeFile(const std::filesystem::path& path, const uint8_t* data, size_t size);
bool writeFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);

// Human readable helpers
std::string formatFileSize(uint64_t bytes);
std::string formatTimestamp(uint64_t unixSeconds);

} // namespace FileUtils
} // namespace SecureVault

