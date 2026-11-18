#include "HttpServer.hpp"
#include "Crypto.hpp"
#include "VaultFormat.hpp"
#include "VaultEntry.hpp"
#include "FileUtils.hpp"
#include "Json.hpp"
#include "Base64.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
typedef SOCKET socket_t;
#define CLOSE_SOCKET(s) closesocket(s)
#define INVALID_SOCK INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
typedef int socket_t;
#define CLOSE_SOCKET(s) close(s)
#define INVALID_SOCK -1
#endif

namespace SecureVault {

namespace fs = std::filesystem;

HttpServer::HttpServer(uint16_t port, bool autoOpenBrowser)
    : m_port(port),
      m_autoOpenBrowser(autoOpenBrowser),
      m_running(false),
      m_serverSocket(0) {
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    m_webRoot = resolveWebRoot();
}

HttpServer::~HttpServer() {
    stop();
#if defined(_WIN32)
    WSACleanup();
#endif
}

void HttpServer::stop() {
    m_running = false;
    if (m_serverSocket != 0 && static_cast<socket_t>(m_serverSocket) != INVALID_SOCK) {
        CLOSE_SOCKET(static_cast<socket_t>(m_serverSocket));
        m_serverSocket = 0;
    }
}

std::string HttpServer::resolveWebRoot() const {
    std::error_code ec;

#if defined(_WIN32)
    char exePathBuf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        fs::path exeDir = fs::path(exePathBuf).parent_path();
        if (fs::exists(exeDir / "index.html", ec)) {
            return exeDir.string();
        }
        if (exeDir.has_parent_path() && fs::exists(exeDir.parent_path() / "index.html", ec)) {
            return exeDir.parent_path().string();
        }
    }
#endif

    fs::path cwd = fs::current_path(ec);
    if (!ec && fs::exists(cwd / "index.html", ec)) {
        return cwd.string();
    }
    if (!ec && cwd.has_parent_path() && fs::exists(cwd.parent_path() / "index.html", ec)) {
        return cwd.parent_path().string();
    }

    return ".";
}

void HttpServer::openBrowser(const std::string& url) const {
#if defined(_WIN32)
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = "open \"" + url + "\"";
    std::system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + url + "\" >/dev/null 2>&1 &";
    std::system(cmd.c_str());
#endif
}

namespace {

std::string getMimeType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos || path.rfind(".htm") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
    if (path.rfind(".vault") != std::string::npos) return "application/octet-stream";
    return "text/plain; charset=utf-8";
}

bool sendAll(socket_t sock, const char* data, size_t length) {
    size_t totalSent = 0;
    while (totalSent < length) {
        size_t remaining = length - totalSent;
        int chunk = static_cast<int>(std::min<size_t>(remaining, 65536));
        int sent = send(sock, data + totalSent, chunk, 0);
        if (sent <= 0) {
            return false;
        }
        totalSent += static_cast<size_t>(sent);
    }
    return true;
}

void sendHttpResponse(
    socket_t sock,
    int statusCode,
    const std::string& statusText,
    const std::string& contentType,
    const uint8_t* body,
    size_t bodyLength
) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << bodyLength << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Passphrase\r\n";
    ss << "Cache-Control: no-cache\r\n";
    ss << "Connection: close\r\n\r\n";

    std::string headerStr = ss.str();
    if (!sendAll(sock, headerStr.data(), headerStr.size())) {
        return;
    }
    if (body && bodyLength > 0) {
        sendAll(sock, reinterpret_cast<const char*>(body), bodyLength);
    }
}

void sendJson(socket_t sock, int statusCode, const JsonValue& json) {
    std::string body = json.dump();
    sendHttpResponse(sock, statusCode, (statusCode == 200 ? "OK" : "Error"), "application/json; charset=utf-8",
                     reinterpret_cast<const uint8_t*>(body.data()), body.size());
}

} // anonymous namespace

void HttpServer::run() {
    socket_t listenSock = INVALID_SOCK;
    bool bound = false;
    uint16_t startPort = m_port;

    for (uint16_t attempt = 0; attempt < 20; ++attempt) {
        uint16_t candidatePort = static_cast<uint16_t>(startPort + attempt);
        listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock == INVALID_SOCK) {
            std::cerr << "Error: Failed to create server socket.\n";
            return;
        }

        int opt = 1;
#if defined(_WIN32)
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        serverAddr.sin_port = htons(candidatePort);

        if (bind(listenSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0) {
            m_port = candidatePort;
            bound = true;
            break;
        }

        CLOSE_SOCKET(listenSock);
        listenSock = INVALID_SOCK;
    }

    if (!bound) {
        std::cerr << "Error: Failed to bind to any port in range " << startPort << "-" << (startPort + 19) << ".\n";
        return;
    }

    if (listen(listenSock, 32) < 0) {
        std::cerr << "Error: Failed to listen on socket.\n";
        CLOSE_SOCKET(listenSock);
        return;
    }

    m_serverSocket = static_cast<uintptr_t>(listenSock);
    m_running = true;

    std::string url = "http://localhost:" + std::to_string(m_port) + "/";

    std::cout << "============================================================\n";
    std::cout << "      SecureVault - Authenticated C++ File Encryption       \n";
    std::cout << "           C++ Native Backend Server Listening              \n";
    std::cout << "============================================================\n\n";
    std::cout << "  Local URL: " << url << "\n";
    std::cout << "  Web Root:  " << m_webRoot << "\n\n";
    std::cout << "Endpoints:\n";
    std::cout << "  GET  /              -> Web UI (index.html)\n";
    std::cout << "  GET  /api/status    -> Server & Crypto Core Status\n";
    std::cout << "  POST /api/pack      -> C++ Archival & Encryption Engine\n";
    std::cout << "  POST /api/unpack    -> C++ Decryption & Extraction Engine\n";
    std::cout << "  POST /api/verify    -> C++ HMAC Integrity Verifier\n";
    std::cout << "  POST /api/info      -> C++ Header Metadata Parser\n\n";

    if (m_autoOpenBrowser) {
        std::cout << "Opening SecureVault UI in your default browser...\n";
        std::thread([this, url]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            this->openBrowser(url);
        }).detach();
    }

    std::cout << "Keep this window open while using the UI. Press Ctrl+C to stop.\n\n";

    while (m_running) {
        sockaddr_in clientAddr{};
#if defined(_WIN32)
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        socket_t clientSock = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

        if (clientSock == INVALID_SOCK) {
            if (!m_running) break;
            continue;
        }

        std::thread([this, clientSock]() {
            this->handleClient(static_cast<uintptr_t>(clientSock));
        }).detach();
    }

    CLOSE_SOCKET(listenSock);
}

void HttpServer::handleClient(uintptr_t clientSockPtr) {
    socket_t sock = static_cast<socket_t>(clientSockPtr);

    std::vector<uint8_t> buffer(65536);
    std::string rawRequest;

    size_t headerEndPos = std::string::npos;
    size_t contentLength = 0;

    // Read headers
    while (headerEndPos == std::string::npos) {
        int bytes = recv(sock, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
        if (bytes <= 0) {
            CLOSE_SOCKET(sock);
            return;
        }
        rawRequest.append(reinterpret_cast<char*>(buffer.data()), static_cast<size_t>(bytes));
        headerEndPos = rawRequest.find("\r\n\r\n");
        if (rawRequest.size() > 65536 && headerEndPos == std::string::npos) {
            CLOSE_SOCKET(sock);
            return;
        }
    }

    std::string headerPart = rawRequest.substr(0, headerEndPos);
    std::string bodyPart = rawRequest.substr(headerEndPos + 4);

    // Parse request line
    std::istringstream hStream(headerPart);
    std::string method, path, httpVersion;
    hStream >> method >> path >> httpVersion;

    // Parse headers
    std::string line;
    while (std::getline(hStream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string headerName = line.substr(0, colon);
            std::string headerVal = line.substr(colon + 1);
            while (!headerVal.empty() && headerVal.front() == ' ') headerVal.erase(headerVal.begin());

            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);
            if (headerName == "content-length") {
                try { contentLength = std::stoull(headerVal); } catch (...) {}
            }
        }
    }

    // Read remaining body bytes if needed
    if (contentLength > bodyPart.size()) {
        bodyPart.reserve(contentLength);
    }
    while (bodyPart.size() < contentLength) {
        size_t remaining = contentLength - bodyPart.size();
        int chunk = static_cast<int>(std::min<size_t>(remaining, buffer.size()));
        int bytes = recv(sock, reinterpret_cast<char*>(buffer.data()), chunk, 0);
        if (bytes <= 0) break;
        bodyPart.append(reinterpret_cast<char*>(buffer.data()), static_cast<size_t>(bytes));
    }

    // Handle OPTIONS (CORS preflight)
    if (method == "OPTIONS") {
        sendHttpResponse(sock, 204, "No Content", "text/plain", nullptr, 0);
        CLOSE_SOCKET(sock);
        return;
    }

    // Clean path query string
    auto queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }

    // =========================================================================
    // API Route: GET /api/status
    // =========================================================================
    if (method == "GET" && path == "/api/status") {
        JsonValue resp = JsonValue::makeObject();
        resp["status"] = "online";
        resp["backend"] = "C++ Native Cryptographic Engine";
        resp["version"] = "1.0.0";
        resp["port"] = static_cast<uint64_t>(m_port);
        resp["cipher"] = "AES-256-CBC";
        resp["kdf"] = "PBKDF2-HMAC-SHA256";
        resp["mac"] = "HMAC-SHA256";
        resp["defaultIterations"] = DEFAULT_PBKDF2_ITERATIONS;
        sendJson(sock, 200, resp);
        CLOSE_SOCKET(sock);
        return;
    }

    // =========================================================================
    // API Route: POST /api/pack
    // =========================================================================
    if (method == "POST" && path == "/api/pack") {
        auto startTime = std::chrono::high_resolution_clock::now();
        try {
            JsonValue req = JsonValue::parse(bodyPart);
            std::string passphrase = req["passphrase"].asString();
            uint32_t iterations = req["iterations"].asUint32(DEFAULT_PBKDF2_ITERATIONS);
            if (iterations < 1000) iterations = 1000;

            const auto& fileArray = req["files"].asArray();
            if (passphrase.empty()) {
                throw std::runtime_error("Passphrase cannot be empty.");
            }
            if (fileArray.empty()) {
                throw std::runtime_error("No files provided for packing.");
            }

            std::vector<VaultEntry> entries;
            uint64_t totalOriginalBytes = 0;

            for (const auto& f : fileArray) {
                std::string relPath = f["path"].asString();
                std::string b64Data = f["data"].asString();
                uint64_t mtime = static_cast<uint64_t>(f["mtime"].asInt64(
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    ).count()
                ));

                relPath = FileUtils::sanitizePath(relPath);
                if (!FileUtils::isSafeRelativePath(relPath)) {
                    continue;
                }

                VaultEntry entry;
                entry.relativePath = relPath;
                entry.data = Base64::decode(b64Data);
                entry.fileSize = entry.data.size();
                entry.modifiedTime = mtime;
                entry.sha256 = Crypto::SHA256::hash(entry.data);

                totalOriginalBytes += entry.fileSize;
                entries.push_back(std::move(entry));
            }

            if (entries.empty()) {
                throw std::runtime_error("No valid safe files to package.");
            }

            // Serialize plaintext payload
            size_t totalPayloadSize = sizeof(uint32_t);
            for (const auto& e : entries) {
                totalPayloadSize += sizeof(uint32_t) + e.relativePath.size();
                totalPayloadSize += sizeof(uint64_t) * 2 + 32 + e.data.size();
            }

            std::vector<uint8_t> plaintext;
            plaintext.reserve(totalPayloadSize);

            auto appendU32 = [&plaintext](uint32_t v) {
                uint8_t b[4] = { static_cast<uint8_t>(v >> 24), static_cast<uint8_t>(v >> 16), static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v) };
                plaintext.insert(plaintext.end(), b, b + 4);
            };
            auto appendU64 = [&plaintext](uint64_t v) {
                uint8_t b[8] = {
                    static_cast<uint8_t>(v >> 56), static_cast<uint8_t>(v >> 48), static_cast<uint8_t>(v >> 40), static_cast<uint8_t>(v >> 32),
                    static_cast<uint8_t>(v >> 24), static_cast<uint8_t>(v >> 16), static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v)
                };
                plaintext.insert(plaintext.end(), b, b + 8);
            };

            appendU32(static_cast<uint32_t>(entries.size()));
            for (const auto& e : entries) {
                appendU32(static_cast<uint32_t>(e.relativePath.size()));
                plaintext.insert(plaintext.end(), e.relativePath.begin(), e.relativePath.end());
                appendU64(e.fileSize);
                appendU64(e.modifiedTime);
                plaintext.insert(plaintext.end(), e.sha256.begin(), e.sha256.end());
                plaintext.insert(plaintext.end(), e.data.begin(), e.data.end());
            }

            // Generate Salt & IV
            auto salt = Crypto::getRandomBytes(SALT_SIZE);
            auto iv = Crypto::getRandomBytes(IV_SIZE);

            // Derive Keys
            std::vector<uint8_t> derivedKeys(64);
            Crypto::PBKDF2::deriveKey(passphrase, salt.data(), salt.size(), iterations, derivedKeys.data(), derivedKeys.size());
            const uint8_t* aesKey = derivedKeys.data();
            const uint8_t* hmacKey = derivedKeys.data() + 32;

            // Encrypt AES-256-CBC
            Crypto::AES256 cipher(aesKey);
            std::vector<uint8_t> ciphertext = cipher.encryptCBC(plaintext.data(), plaintext.size(), iv.data());
            Crypto::secureZeroContainer(plaintext);

            // Create Header
            VaultHeader header = VaultFormat::createHeader(iterations, salt.data(), iv.data(), ciphertext.size());

            // Compute HMAC-SHA256
            auto headerBytes = VaultFormat::getHeaderBytesForMAC(header);
            Crypto::HMAC_SHA256 mac(hmacKey, 32);
            mac.update(headerBytes);
            mac.update(ciphertext);
            auto tag = mac.finalize();
            std::memcpy(header.hmac, tag.data(), HMAC_SIZE);
            Crypto::secureZeroContainer(derivedKeys);

            // Combine Header + Ciphertext
            std::vector<uint8_t> finalVault(sizeof(header) + ciphertext.size());
            std::memcpy(finalVault.data(), &header, sizeof(header));
            std::memcpy(finalVault.data() + sizeof(header), ciphertext.data(), ciphertext.size());

            auto endTime = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();

            std::cout << "[API] POST /api/pack -> " << entries.size() << " file(s), "
                      << finalVault.size() << " bytes (" << duration << " ms)\n";

            JsonValue resp = JsonValue::makeObject();
            resp["success"] = true;
            resp["fileCount"] = static_cast<uint64_t>(entries.size());
            resp["originalBytes"] = totalOriginalBytes;
            resp["vaultBytes"] = static_cast<uint64_t>(finalVault.size());
            resp["durationMs"] = duration;
            resp["vaultData"] = Base64::encode(finalVault);

            sendJson(sock, 200, resp);
        } catch (const std::exception& e) {
            JsonValue err = JsonValue::makeObject();
            err["success"] = false;
            err["error"] = e.what();
            sendJson(sock, 400, err);
        }
        CLOSE_SOCKET(sock);
        return;
    }

    // =========================================================================
    // API Route: POST /api/unpack
    // =========================================================================
    if (method == "POST" && path == "/api/unpack") {
        auto startTime = std::chrono::high_resolution_clock::now();
        try {
            JsonValue req = JsonValue::parse(bodyPart);
            std::string passphrase = req["passphrase"].asString();
            std::string b64Vault = req["vaultData"].asString();

            if (passphrase.empty()) throw std::runtime_error("Passphrase cannot be empty.");
            if (b64Vault.empty()) throw std::runtime_error("Vault data cannot be empty.");

            std::vector<uint8_t> rawVault = Base64::decode(b64Vault);
            if (rawVault.size() < sizeof(VaultHeader)) {
                throw std::runtime_error("Vault data is smaller than header specification.");
            }

            VaultHeader header{};
            std::memcpy(&header, rawVault.data(), sizeof(VaultHeader));

            if (!VaultFormat::validateHeader(header)) {
                throw std::runtime_error("Invalid or unsupported vault header format.");
            }

            uint64_t expectedPayload = rawVault.size() - sizeof(VaultHeader);
            if (header.ciphertextLength != expectedPayload) {
                throw std::runtime_error("Payload size mismatch in vault archive.");
            }

            const uint8_t* ciphertextPtr = rawVault.data() + sizeof(VaultHeader);

            // Derive Keys
            std::vector<uint8_t> derivedKeys(64);
            Crypto::PBKDF2::deriveKey(passphrase, header.salt, SALT_SIZE, header.kdfIterations, derivedKeys.data(), derivedKeys.size());
            const uint8_t* aesKey = derivedKeys.data();
            const uint8_t* hmacKey = derivedKeys.data() + 32;

            // Verify HMAC
            auto headerBytes = VaultFormat::getHeaderBytesForMAC(header);
            Crypto::HMAC_SHA256 mac(hmacKey, 32);
            mac.update(headerBytes);
            mac.update(ciphertextPtr, static_cast<size_t>(header.ciphertextLength));
            auto calculatedTag = mac.finalize();

            if (!Crypto::HMAC_SHA256::verify(calculatedTag.data(), header.hmac, HMAC_SIZE)) {
                Crypto::secureZeroContainer(derivedKeys);
                std::cout << "[API] POST /api/unpack -> Authentication failed\n";
                JsonValue err = JsonValue::makeObject();
                err["success"] = false;
                err["error"] = "Authentication failed: Invalid key or vault data corrupted.";
                sendJson(sock, 200, err);
                CLOSE_SOCKET(sock);
                return;
            }

            // Decrypt AES-256-CBC
            Crypto::AES256 cipher(aesKey);
            std::vector<uint8_t> plaintext;
            bool decOk = cipher.decryptCBC(ciphertextPtr, static_cast<size_t>(header.ciphertextLength), header.iv, plaintext);
            Crypto::secureZeroContainer(derivedKeys);

            if (!decOk) {
                throw std::runtime_error("Decryption padding error.");
            }

            // Deserialize catalog
            if (plaintext.size() < 4) throw std::runtime_error("Corrupted plaintext manifest.");
            size_t offset = 0;

            auto readU32 = [&plaintext, &offset](uint32_t& v) -> bool {
                if (offset + 4 > plaintext.size()) return false;
                v = (static_cast<uint32_t>(plaintext[offset]) << 24) |
                    (static_cast<uint32_t>(plaintext[offset + 1]) << 16) |
                    (static_cast<uint32_t>(plaintext[offset + 2]) << 8) |
                    static_cast<uint32_t>(plaintext[offset + 3]);
                offset += 4;
                return true;
            };

            auto readU64 = [&plaintext, &offset](uint64_t& v) -> bool {
                if (offset + 8 > plaintext.size()) return false;
                v = (static_cast<uint64_t>(plaintext[offset]) << 56) |
                    (static_cast<uint64_t>(plaintext[offset + 1]) << 48) |
                    (static_cast<uint64_t>(plaintext[offset + 2]) << 40) |
                    (static_cast<uint64_t>(plaintext[offset + 3]) << 32) |
                    (static_cast<uint64_t>(plaintext[offset + 4]) << 24) |
                    (static_cast<uint64_t>(plaintext[offset + 5]) << 16) |
                    (static_cast<uint64_t>(plaintext[offset + 6]) << 8) |
                    static_cast<uint64_t>(plaintext[offset + 7]);
                offset += 8;
                return true;
            };

            uint32_t count = 0;
            if (!readU32(count)) throw std::runtime_error("Failed to read entry count.");

            JsonValue filesArr = JsonValue::makeArray();
            uint64_t totalBytes = 0;

            for (uint32_t i = 0; i < count; ++i) {
                uint32_t pathLen = 0;
                if (!readU32(pathLen) || offset + pathLen > plaintext.size()) throw std::runtime_error("Corrupted path length.");
                std::string relPath(reinterpret_cast<const char*>(plaintext.data() + offset), pathLen);
                offset += pathLen;

                uint64_t fileSize = 0;
                if (!readU64(fileSize)) throw std::runtime_error("Corrupted file size.");

                uint64_t mtime = 0;
                if (!readU64(mtime)) throw std::runtime_error("Corrupted modification time.");

                if (offset + 32 > plaintext.size()) throw std::runtime_error("Corrupted file hash.");
                std::string hashHex = Crypto::SHA256::toHexString(plaintext.data() + offset, 32);
                const uint8_t* expectedHashPtr = plaintext.data() + offset;
                offset += 32;

                if (offset + fileSize > plaintext.size()) throw std::runtime_error("Corrupted payload size.");
                const uint8_t* dataPtr = plaintext.data() + offset;

                // Verify file SHA-256
                auto actualHash = Crypto::SHA256::hash(dataPtr, static_cast<size_t>(fileSize));
                if (std::memcmp(actualHash.data(), expectedHashPtr, 32) != 0) {
                    throw std::runtime_error("SHA-256 verification failed for file: " + relPath);
                }

                JsonValue fileObj = JsonValue::makeObject();
                fileObj["path"] = relPath;
                fileObj["size"] = fileSize;
                fileObj["mtime"] = mtime;
                fileObj["sha256"] = hashHex;
                fileObj["data"] = Base64::encode(dataPtr, static_cast<size_t>(fileSize));

                totalBytes += fileSize;
                offset += static_cast<size_t>(fileSize);
                filesArr.push_back(fileObj);
            }

            Crypto::secureZeroContainer(plaintext);

            auto endTime = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(endTime - startTime).count();

            std::cout << "[API] POST /api/unpack -> " << count << " file(s) restored (" << duration << " ms)\n";

            JsonValue resp = JsonValue::makeObject();
            resp["success"] = true;
            resp["fileCount"] = static_cast<uint64_t>(count);
            resp["totalBytes"] = totalBytes;
            resp["durationMs"] = duration;
            resp["files"] = filesArr;

            sendJson(sock, 200, resp);
        } catch (const std::exception& e) {
            JsonValue err = JsonValue::makeObject();
            err["success"] = false;
            err["error"] = e.what();
            sendJson(sock, 400, err);
        }
        CLOSE_SOCKET(sock);
        return;
    }

    // =========================================================================
    // API Route: POST /api/verify
    // =========================================================================
    if (method == "POST" && path == "/api/verify") {
        try {
            JsonValue req = JsonValue::parse(bodyPart);
            std::string passphrase = req["passphrase"].asString();
            std::string b64Vault = req["vaultData"].asString();

            std::vector<uint8_t> rawVault = Base64::decode(b64Vault);
            if (rawVault.size() < sizeof(VaultHeader)) throw std::runtime_error("Vault size too small.");

            VaultHeader header{};
            std::memcpy(&header, rawVault.data(), sizeof(VaultHeader));

            if (!VaultFormat::validateHeader(header)) throw std::runtime_error("Invalid vault header.");

            uint64_t actualPayloadLen = rawVault.size() - sizeof(VaultHeader);
            if (header.ciphertextLength != actualPayloadLen) {
                throw std::runtime_error("Vault payload size mismatch.");
            }

            std::vector<uint8_t> derivedKeys(64);
            Crypto::PBKDF2::deriveKey(passphrase, header.salt, SALT_SIZE, header.kdfIterations, derivedKeys.data(), derivedKeys.size());
            const uint8_t* hmacKey = derivedKeys.data() + 32;

            auto headerBytes = VaultFormat::getHeaderBytesForMAC(header);
            Crypto::HMAC_SHA256 mac(hmacKey, 32);
            mac.update(headerBytes);
            mac.update(rawVault.data() + sizeof(VaultHeader), static_cast<size_t>(header.ciphertextLength));
            auto calculatedTag = mac.finalize();

            bool verified = Crypto::HMAC_SHA256::verify(calculatedTag.data(), header.hmac, HMAC_SIZE);
            Crypto::secureZeroContainer(derivedKeys);

            std::cout << "[API] POST /api/verify -> " << (verified ? "PASS" : "FAIL") << "\n";

            JsonValue resp = JsonValue::makeObject();
            resp["success"] = true;
            resp["verified"] = verified;
            resp["message"] = verified ? "Key is valid and archive integrity is intact." : "Authentication failed.";
            sendJson(sock, 200, resp);
        } catch (const std::exception& e) {
            JsonValue err = JsonValue::makeObject();
            err["success"] = false;
            err["error"] = e.what();
            sendJson(sock, 400, err);
        }
        CLOSE_SOCKET(sock);
        return;
    }

    // =========================================================================
    // API Route: POST /api/info
    // =========================================================================
    if (method == "POST" && path == "/api/info") {
        try {
            JsonValue req = JsonValue::parse(bodyPart);
            std::string b64Vault = req["vaultData"].asString();
            std::vector<uint8_t> rawVault = Base64::decode(b64Vault);
            if (rawVault.size() < sizeof(VaultHeader)) throw std::runtime_error("Vault size too small.");

            VaultHeader header{};
            std::memcpy(&header, rawVault.data(), sizeof(VaultHeader));
            if (!VaultFormat::validateHeader(header)) throw std::runtime_error("Invalid vault header.");

            JsonValue resp = JsonValue::makeObject();
            resp["success"] = true;
            resp["magic"] = std::string(header.magic, 6);
            resp["version"] = header.version;
            resp["cipherId"] = header.cipherId;
            resp["cipherName"] = "AES-256-CBC";
            resp["kdfId"] = header.kdfId;
            resp["kdfName"] = "PBKDF2-HMAC-SHA256";
            resp["kdfIterations"] = header.kdfIterations;
            resp["ciphertextLength"] = header.ciphertextLength;
            resp["salt"] = Crypto::SHA256::toHexString(header.salt, SALT_SIZE);
            resp["iv"] = Crypto::SHA256::toHexString(header.iv, IV_SIZE);
            resp["hmac"] = Crypto::SHA256::toHexString(header.hmac, HMAC_SIZE);

            sendJson(sock, 200, resp);
        } catch (const std::exception& e) {
            JsonValue err = JsonValue::makeObject();
            err["success"] = false;
            err["error"] = e.what();
            sendJson(sock, 400, err);
        }
        CLOSE_SOCKET(sock);
        return;
    }

    // =========================================================================
    // Static File Serving (resolved relative to executable directory / web root)
    // =========================================================================
    std::string filePath = (path == "/" || path.empty()) ? "index.html" : path.substr(1);
    if (!FileUtils::isSafeRelativePath(filePath)) {
        std::string forbidden = "403 Forbidden";
        sendHttpResponse(sock, 403, "Forbidden", "text/plain", reinterpret_cast<const uint8_t*>(forbidden.data()), forbidden.size());
        CLOSE_SOCKET(sock);
        return;
    }

    fs::path targetFile = fs::path(m_webRoot) / filePath;

    if (fs::exists(targetFile) && fs::is_regular_file(targetFile)) {
        try {
            auto fileBytes = FileUtils::readFile(targetFile);
            std::string mime = getMimeType(filePath);
            sendHttpResponse(sock, 200, "OK", mime, fileBytes.data(), fileBytes.size());
        } catch (...) {
            std::string notFound = "404 Not Found";
            sendHttpResponse(sock, 404, "Not Found", "text/plain", reinterpret_cast<const uint8_t*>(notFound.data()), notFound.size());
        }
    } else {
        std::string notFound = "404 Not Found";
        sendHttpResponse(sock, 404, "Not Found", "text/plain", reinterpret_cast<const uint8_t*>(notFound.data()), notFound.size());
    }

    CLOSE_SOCKET(sock);
}

} // namespace SecureVault
