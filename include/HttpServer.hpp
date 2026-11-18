#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <cstdint>

namespace SecureVault {

class HttpServer {
public:
    explicit HttpServer(uint16_t port = 8080, bool autoOpenBrowser = false);
    ~HttpServer();

    // Starts the HTTP server on the configured port (with fallback if port is in use).
    // Blocks until stop() is called or Ctrl+C.
    void run();

    // Stops the running server
    void stop();

    // Returns the active bound port
    uint16_t getPort() const { return m_port; }

private:
    uint16_t m_port;
    bool m_autoOpenBrowser;
    std::atomic<bool> m_running;
    uintptr_t m_serverSocket;
    std::string m_webRoot;

    void handleClient(uintptr_t clientSocket);
    std::string resolveWebRoot() const;
    void openBrowser(const std::string& url) const;
};

} // namespace SecureVault
