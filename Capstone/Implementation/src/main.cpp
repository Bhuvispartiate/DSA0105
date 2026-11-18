#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <algorithm>

#include "VaultWriter.hpp"
#include "VaultReader.hpp"
#include "VaultFormat.hpp"
#include "FileUtils.hpp"
#include "Crypto.hpp"
#include "HttpServer.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;
using namespace SecureVault;

// Secure password prompt with disabled console echo
std::string readPassword(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string password;

#if defined(_WIN32)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    std::getline(std::cin, password);

    SetConsoleMode(hStdin, mode);
    std::cout << std::endl;
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
#endif

    return password;
}

std::string ensureVaultExtension(const std::string& path) {
    if (path.size() >= 6 && path.substr(path.size() - 6) == ".vault") {
        return path;
    }
    return path + ".vault";
}

void printBanner() {
    std::cout << "============================================================\n";
    std::cout << "      SecureVault - Authenticated C++ File Encryption       \n";
    std::cout << "           AES-256-CBC | HMAC-SHA256 | PBKDF2               \n";
    std::cout << "============================================================\n\n";
}

void printUsage(const char* progName) {
    std::cout << "Usage:\n";
    std::cout << "  " << progName << "                                 Launch Web UI with C++ backend (default)\n";
    std::cout << "  " << progName << " ui     [port] [--no-open]       Launch Web UI on specified port (default: 8080)\n";
    std::cout << "  " << progName << " serve  [port] [--no-open]       Start C++ native HTTP backend server\n";
    std::cout << "  " << progName << " cli                             Launch interactive terminal menu\n";
    std::cout << "  " << progName << " pack   -o <vault-file> [-k <key>] [--iterations <n>] <files/dirs...>\n";
    std::cout << "  " << progName << " unpack -i <vault-file> [-o <dest-dir>] [-k <key>]\n";
    std::cout << "  " << progName << " list   -i <vault-file> [-k <key>]\n";
    std::cout << "  " << progName << " verify -i <vault-file> [-k <key>]\n";
    std::cout << "  " << progName << " info   -i <vault-file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o, --output <path>       Output vault file or destination folder\n";
    std::cout << "  -i, --input  <path>       Input vault file\n";
    std::cout << "  -k, --key    <password>   Passphrase/Key (prompted securely if omitted)\n";
    std::cout << "  --iterations <count>      PBKDF2 iteration count (default: 100,000)\n";
    std::cout << "  --no-open                 Do not automatically open the browser when serving UI\n";
    std::cout << "  -h, --help                Show this help message\n";
}

int handlePack(const std::string& outVault, const std::string& key, uint32_t iterations, const std::vector<std::string>& sources) {
    if (outVault.empty()) {
        std::cerr << "Error: Output vault path is required (-o <vault.vault>).\n";
        return 1;
    }
    if (sources.empty()) {
        std::cerr << "Error: At least one input file or directory must be specified.\n";
        return 1;
    }

    std::string vaultFile = ensureVaultExtension(outVault);
    std::string passphrase = key;

    if (passphrase.empty()) {
        passphrase = readPassword("Enter encryption key: ");
        if (passphrase.empty()) {
            std::cerr << "Error: Key cannot be empty.\n";
            return 1;
        }
        std::string confirm = readPassword("Confirm encryption key: ");
        if (passphrase != confirm) {
            std::cerr << "Error: Keys do not match. Aborting.\n";
            return 1;
        }
    }

    try {
        VaultWriter writer(vaultFile, passphrase);
        writer.setKdfIterations(iterations);
        writer.addSources(sources);

        std::cout << "Encrypting and packing into: " << vaultFile << "\n";
        PackStats stats = writer.pack([](const std::string& file, size_t index, size_t total) {
            std::cout << "  [" << index << "/" << total << "] Adding: " << file << "\n";
        });

        std::cout << "\nVault created successfully!\n";
        std::cout << "  Total files:      " << stats.fileCount << "\n";
        std::cout << "  Original size:    " << FileUtils::formatFileSize(stats.totalOriginalBytes) << "\n";
        std::cout << "  Encrypted size:   " << FileUtils::formatFileSize(stats.totalEncryptedBytes) << "\n";
        std::cout << "  Vault file size:  " << FileUtils::formatFileSize(stats.vaultFileSizeBytes) << "\n";
        std::cout << "  Time elapsed:     " << std::fixed << std::setprecision(1) << stats.durationMs << " ms\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << "\n";
        return 1;
    }
}

int handleUnpack(const std::string& inVault, const std::string& outDir, const std::string& key) {
    if (inVault.empty()) {
        std::cerr << "Error: Input vault path is required (-i <vault.vault>).\n";
        return 1;
    }

    std::string targetDir = outDir.empty() ? "./extracted" : outDir;
    std::string passphrase = key;

    if (passphrase.empty()) {
        passphrase = readPassword("Enter decryption key: ");
    }

    try {
        VaultReader reader(inVault);
        std::cout << "Verifying authentication tag and unlocking vault...\n";

        if (!reader.unlock(passphrase)) {
            std::cerr << "Decryption failed: Invalid key or vault data corrupted/tampered.\n";
            return 2;
        }

        std::cout << "Unlocking successful. Extracting files to: " << targetDir << "\n";
        UnpackStats stats = reader.extractAll(targetDir, [](const std::string& file, size_t index, size_t total) {
            std::cout << "  [" << index << "/" << total << "] Restored: " << file << "\n";
        });

        std::cout << "\nExtraction completed successfully!\n";
        std::cout << "  Files extracted:  " << stats.fileCount << "\n";
        std::cout << "  Extracted bytes:  " << FileUtils::formatFileSize(stats.totalExtractedBytes) << "\n";
        std::cout << "  Time elapsed:     " << std::fixed << std::setprecision(1) << stats.durationMs << " ms\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Extraction failed: " << e.what() << "\n";
        return 1;
    }
}

int handleList(const std::string& inVault, const std::string& key) {
    if (inVault.empty()) {
        std::cerr << "Error: Input vault path is required (-i <vault.vault>).\n";
        return 1;
    }

    std::string passphrase = key;
    if (passphrase.empty()) {
        passphrase = readPassword("Enter decryption key: ");
    }

    try {
        VaultReader reader(inVault);
        if (!reader.unlock(passphrase)) {
            std::cerr << "Authentication failed: Incorrect key or corrupted vault.\n";
            return 2;
        }

        const auto& entries = reader.getEntries();
        std::cout << "\nVault Catalog (" << entries.size() << " entries):\n";
        std::cout << "---------------------------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(38) << "Relative Path"
                  << std::right << std::setw(14) << "Size"
                  << std::setw(22) << "Modified Date"
                  << "  " << "SHA-256 (prefix)\n";
        std::cout << "---------------------------------------------------------------------------------------------------------\n";

        uint64_t totalBytes = 0;
        for (const auto& entry : entries) {
            std::string hashHex = Crypto::SHA256::toHexString(entry.sha256.data(), 8) + "...";
            std::string dateStr = FileUtils::formatTimestamp(entry.modifiedTime);
            std::string sizeStr = FileUtils::formatFileSize(entry.fileSize);

            std::cout << std::left << std::setw(38) << entry.relativePath
                      << std::right << std::setw(14) << sizeStr
                      << std::setw(22) << dateStr
                      << "  " << hashHex << "\n";
            totalBytes += entry.fileSize;
        }
        std::cout << "---------------------------------------------------------------------------------------------------------\n";
        std::cout << "Total Archive Size: " << FileUtils::formatFileSize(totalBytes) << " across " << entries.size() << " files.\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int handleVerify(const std::string& inVault, const std::string& key) {
    if (inVault.empty()) {
        std::cerr << "Error: Input vault path is required (-i <vault.vault>).\n";
        return 1;
    }

    std::string passphrase = key;
    if (passphrase.empty()) {
        passphrase = readPassword("Enter decryption key to verify: ");
    }

    try {
        VaultReader reader(inVault);
        std::cout << "Computing HMAC-SHA256 over envelope and ciphertext...\n";
        if (reader.verifyPassphrase(passphrase)) {
            std::cout << "[PASS] Authentication verified: Key is valid and archive integrity is intact.\n";
            return 0;
        } else {
            std::cout << "[FAIL] Verification failed: Incorrect key or archive has been tampered with.\n";
            return 2;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during verification: " << e.what() << "\n";
        return 1;
    }
}

int handleInfo(const std::string& inVault) {
    if (inVault.empty()) {
        std::cerr << "Error: Input vault path is required (-i <vault.vault>).\n";
        return 1;
    }

    try {
        VaultReader reader(inVault);
        const VaultHeader& h = reader.getHeader();

        std::string magicStr(h.magic, sizeof(h.magic));
        std::cout << "\nVault Envelope Metadata:\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << "  Magic Signature:    " << magicStr << "\n";
        std::cout << "  Format Version:     " << h.version << "\n";
        std::cout << "  Cipher Algorithm:   AES-256-CBC (ID " << h.cipherId << ")\n";
        std::cout << "  KDF Algorithm:      PBKDF2-HMAC-SHA256 (ID " << h.kdfId << ")\n";
        std::cout << "  PBKDF2 Iterations:  " << h.kdfIterations << " rounds\n";
        std::cout << "  Random Salt (hex):  " << Crypto::SHA256::toHexString(h.salt, SALT_SIZE) << "\n";
        std::cout << "  Random IV (hex):    " << Crypto::SHA256::toHexString(h.iv, IV_SIZE) << "\n";
        std::cout << "  Ciphertext Size:    " << FileUtils::formatFileSize(h.ciphertextLength) << " (" << h.ciphertextLength << " bytes)\n";
        std::cout << "  HMAC-SHA256 Tag:    " << Crypto::SHA256::toHexString(h.hmac, HMAC_SIZE) << "\n";
        std::cout << "--------------------------------------------------\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error reading vault metadata: " << e.what() << "\n";
        return 1;
    }
}

void runInteractive() {
    printBanner();
    while (true) {
        std::cout << "Choose an operation:\n";
        std::cout << "  1. Pack files/folders into a .vault archive (Encrypt)\n";
        std::cout << "  2. Unpack a .vault archive (Decrypt)\n";
        std::cout << "  3. List contents of a .vault archive\n";
        std::cout << "  4. Verify vault key & integrity\n";
        std::cout << "  5. Inspect vault envelope metadata\n";
        std::cout << "  6. Start C++ Native Backend Server (Web UI)\n";
        std::cout << "  7. Exit\n";
        std::cout << "\nEnter choice (1-7): ";

        std::string choice;
        if (!std::getline(std::cin, choice)) break;

        if (choice == "1") {
            std::cout << "Enter target .vault filename to create (e.g. secure.vault): ";
            std::string outVault;
            std::getline(std::cin, outVault);

            std::cout << "Enter files or directories to include (separated by space): ";
            std::string sourcesLine;
            std::getline(std::cin, sourcesLine);

            std::vector<std::string> sources;
            std::istringstream iss(sourcesLine);
            std::string token;
            while (iss >> token) {
                sources.push_back(token);
            }

            handlePack(outVault, "", DEFAULT_PBKDF2_ITERATIONS, sources);
        } else if (choice == "2") {
            std::cout << "Enter path to .vault file: ";
            std::string inVault;
            std::getline(std::cin, inVault);

            std::cout << "Enter destination directory (default: ./extracted): ";
            std::string outDir;
            std::getline(std::cin, outDir);

            handleUnpack(inVault, outDir, "");
        } else if (choice == "3") {
            std::cout << "Enter path to .vault file: ";
            std::string inVault;
            std::getline(std::cin, inVault);

            handleList(inVault, "");
        } else if (choice == "4") {
            std::cout << "Enter path to .vault file: ";
            std::string inVault;
            std::getline(std::cin, inVault);

            handleVerify(inVault, "");
        } else if (choice == "5") {
            std::cout << "Enter path to .vault file: ";
            std::string inVault;
            std::getline(std::cin, inVault);

            handleInfo(inVault);
        } else if (choice == "6") {
            std::cout << "Enter port to listen on (default: 8080): ";
            std::string portStr;
            std::getline(std::cin, portStr);
            uint16_t port = 8080;
            if (!portStr.empty()) {
                try { port = static_cast<uint16_t>(std::stoul(portStr)); } catch (...) { port = 8080; }
            }
            HttpServer server(port, true);
            server.run();
        } else if (choice == "7" || choice == "q" || choice == "exit") {
            std::cout << "Exiting SecureVault.\n";
            break;
        } else {
            std::cout << "Invalid choice. Please select 1 through 7.\n";
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        // Default: Launch the C++ backend server and open the Web UI in browser
        HttpServer server(8080, true);
        server.run();
        return 0;
    }

    std::string command = argv[1];

    if (command == "-h" || command == "--help" || command == "help") {
        printBanner();
        printUsage(argv[0]);
        return 0;
    }

    if (command == "cli" || command == "--interactive" || command == "-i_interactive") {
        runInteractive();
        return 0;
    }

    if (command == "ui" || command == "serve" || command == "server" || command == "web") {
        uint16_t port = 8080;
        bool openBrowser = true;
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--no-open" || arg == "--headless") {
                openBrowser = false;
            } else {
                try { port = static_cast<uint16_t>(std::stoul(arg)); } catch (...) {}
            }
        }
        HttpServer server(port, openBrowser);
        server.run();
        return 0;
    }

    std::string inVault;
    std::string outVault;
    std::string outDir;
    std::string key;
    uint32_t iterations = DEFAULT_PBKDF2_ITERATIONS;
    std::vector<std::string> sources;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outVault = argv[++i];
            outDir = outVault;
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            inVault = argv[++i];
        } else if ((arg == "-k" || arg == "--key" || arg == "-p" || arg == "--password") && i + 1 < argc) {
            key = argv[++i];
        } else if (arg == "--iterations" && i + 1 < argc) {
            iterations = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            sources.push_back(arg);
        }
    }

    if (command == "pack" || command == "create" || command == "encrypt") {
        return handlePack(outVault, key, iterations, sources);
    } else if (command == "unpack" || command == "extract" || command == "decrypt") {
        return handleUnpack(inVault, outDir, key);
    } else if (command == "list" || command == "ls") {
        return handleList(inVault, key);
    } else if (command == "verify" || command == "check") {
        return handleVerify(inVault, key);
    } else if (command == "info" || command == "header") {
        return handleInfo(inVault);
    } else {
        std::cerr << "Unknown command: " << command << "\n\n";
        printUsage(argv[0]);
        return 1;
    }
}

