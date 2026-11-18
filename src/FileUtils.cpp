#include "FileUtils.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace SecureVault {
namespace FileUtils {

namespace fs = std::filesystem;

bool isSafeRelativePath(const std::string& path) {
    if (path.empty()) return false;

    // Check for drive letters (e.g. C:)
    if (path.size() >= 2 && path[1] == ':' && std::isalpha(static_cast<unsigned char>(path[0]))) {
        return false;
    }

    // Check for leading slash or backslash
    if (path.front() == '/' || path.front() == '\\') {
        return false;
    }

    fs::path p(path);
    if (p.is_absolute()) {
        return false;
    }

    for (const auto& part : p) {
        if (part == ".." || part == ".") {
            return false;
        }
    }

    return true;
}

std::string sanitizePath(const std::string& path) {
    std::string clean = path;
    std::replace(clean.begin(), clean.end(), '\\', '/');

    // Remove any leading slashes
    while (!clean.empty() && (clean.front() == '/' || clean.front() == ' ')) {
        clean.erase(clean.begin());
    }

    // Collapse multiple consecutive slashes
    std::string result;
    bool lastWasSlash = false;
    for (char c : clean) {
        if (c == '/') {
            if (!lastWasSlash) {
                result += c;
                lastWasSlash = true;
            }
        } else {
            result += c;
            lastWasSlash = false;
        }
    }
    return result;
}

std::vector<FileItem> collectFiles(const std::vector<std::string>& inputPaths) {
    std::vector<FileItem> items;

    for (const auto& rawInput : inputPaths) {
        fs::path inputPath(rawInput);

        std::error_code ec;
        if (!fs::exists(inputPath, ec)) {
            continue;
        }

        if (fs::is_regular_file(inputPath, ec)) {
            FileItem item;
            item.diskPath = fs::absolute(inputPath);
            item.vaultRelativePath = sanitizePath(inputPath.filename().string());
            item.fileSize = fs::file_size(inputPath, ec);
            auto ftime = fs::last_write_time(inputPath, ec);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            item.modifiedTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count());
            items.push_back(item);
        } else if (fs::is_directory(inputPath, ec)) {
            fs::path baseDir = inputPath.filename().empty() ? inputPath.parent_path().filename() : inputPath.filename();

            for (const auto& dirEntry : fs::recursive_directory_iterator(inputPath, fs::directory_options::skip_permission_denied, ec)) {
                if (dirEntry.is_regular_file(ec)) {
                    fs::path rel = fs::relative(dirEntry.path(), inputPath, ec);
                    fs::path storedPath = baseDir / rel;

                    FileItem item;
                    item.diskPath = fs::absolute(dirEntry.path());
                    item.vaultRelativePath = sanitizePath(storedPath.string());
                    item.fileSize = dirEntry.file_size(ec);

                    auto ftime = dirEntry.last_write_time(ec);
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    item.modifiedTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count());
                    items.push_back(item);
                }
            }
        }
    }

    return items;
}

std::vector<uint8_t> readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + path.string());
    }

    std::streamsize size = file.tellg();
    if (size < 0) {
        throw std::runtime_error("Failed to determine size of file: " + path.string());
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (size > 0) {
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            throw std::runtime_error("Error reading file: " + path.string());
        }
    }
    return buffer;
}

bool writeFile(const fs::path& path, const uint8_t* data, size_t size) {
    std::error_code ec;
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path(), ec);
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }

    if (size > 0 && data != nullptr) {
        file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    }
    return file.good();
}

bool writeFile(const fs::path& path, const std::vector<uint8_t>& data) {
    return writeFile(path, data.data(), data.size());
}

std::string formatFileSize(uint64_t bytes) {
    constexpr double KB = 1024.0;
    constexpr double MB = KB * 1024.0;
    constexpr double GB = MB * 1024.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (bytes >= GB) {
        oss << (static_cast<double>(bytes) / GB) << " GB";
    } else if (bytes >= MB) {
        oss << (static_cast<double>(bytes) / MB) << " MB";
    } else if (bytes >= KB) {
        oss << (static_cast<double>(bytes) / KB) << " KB";
    } else {
        oss << bytes << " bytes";
    }
    return oss.str();
}

std::string formatTimestamp(uint64_t unixSeconds) {
    std::time_t t = static_cast<std::time_t>(unixSeconds);
    std::tm tmBuffer{};
#if defined(_WIN32)
    localtime_s(&tmBuffer, &t);
#else
    localtime_r(&t, &tmBuffer);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuffer, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace FileUtils
} // namespace SecureVault

