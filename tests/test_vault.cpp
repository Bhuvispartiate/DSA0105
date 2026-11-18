#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>

#include "Crypto.hpp"
#include "VaultFormat.hpp"
#include "VaultWriter.hpp"
#include "VaultReader.hpp"
#include "FileUtils.hpp"

namespace fs = std::filesystem;
using namespace SecureVault;

int g_passedTests = 0;
int g_failedTests = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAIL] " << msg << " (line " << __LINE__ << ")\n"; \
            g_failedTests++; \
            return; \
        } \
    } while(0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "[RUNNING] " << #fn << "..." << std::flush; \
        int oldFail = g_failedTests; \
        fn(); \
        if (g_failedTests == oldFail) { \
            std::cout << " [PASS]\n"; \
            g_passedTests++; \
        } \
    } while(0)

void test_sha256_known_vectors() {
    // Vector 1: Empty string
    // e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    auto h1 = Crypto::SHA256::hash(nullptr, 0);
    std::string hex1 = Crypto::SHA256::toHexString(h1.data(), 32);
    TEST_ASSERT(hex1 == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "SHA256 of empty string mismatch");

    // Vector 2: "abc"
    // ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
    std::string msg2 = "abc";
    auto h2 = Crypto::SHA256::hash(reinterpret_cast<const uint8_t*>(msg2.data()), msg2.size());
    std::string hex2 = Crypto::SHA256::toHexString(h2.data(), 32);
    TEST_ASSERT(hex2 == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA256 of 'abc' mismatch");
}

void test_hmac_sha256() {
    // RFC 4231 Test Case 2: key = "Jefe", data = "what do ya want for nothing?"
    // Expected: 5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843
    std::string key = "Jefe";
    std::string data = "what do ya want for nothing?";
    auto tag = Crypto::HMAC_SHA256::compute(
        reinterpret_cast<const uint8_t*>(key.data()), key.size(),
        reinterpret_cast<const uint8_t*>(data.data()), data.size()
    );
    std::string hex = Crypto::SHA256::toHexString(tag.data(), 32);
    TEST_ASSERT(hex == "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", "HMAC-SHA256 RFC 4231 Case 2 mismatch");
}

void test_aes256_cbc_roundtrip() {
    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t iv[16] = {
        0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
        0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf
    };

    std::string secretMessage = "Antigravity Secure Vault Test Payload with arbitrary length 1234567890!";
    Crypto::AES256 cipher(key);

    auto ciphertext = cipher.encryptCBC(
        reinterpret_cast<const uint8_t*>(secretMessage.data()),
        secretMessage.size(),
        iv
    );

    TEST_ASSERT(!ciphertext.empty(), "Ciphertext should not be empty");
    TEST_ASSERT(ciphertext.size() % 16 == 0, "Ciphertext size must be multiple of AES block size (16)");

    std::vector<uint8_t> decrypted;
    bool success = cipher.decryptCBC(ciphertext.data(), ciphertext.size(), iv, decrypted);
    TEST_ASSERT(success, "Decryption returned failure");

    std::string decryptedStr(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
    TEST_ASSERT(decryptedStr == secretMessage, "Decrypted text does not match original plaintext");
}

void test_single_file_vault_pack_unpack() {
    fs::path testDir = "./temp_test_single";
    fs::create_directories(testDir);

    fs::path srcFile = testDir / "hello.txt";
    std::string content = "Confidential Top-Secret Documents\nCreated on 2026-10-30\nAntigravity C++ Vault.";
    FileUtils::writeFile(srcFile, reinterpret_cast<const uint8_t*>(content.data()), content.size());

    fs::path vaultFile = testDir / "archive.vault";
    std::string password = "StrongPassword@2026";

    // Pack
    {
        VaultWriter writer(vaultFile.string(), password);
        writer.setKdfIterations(5000); // lower iterations for rapid test suite execution
        writer.addSource(srcFile.string());
        PackStats stats = writer.pack();
        TEST_ASSERT(stats.fileCount == 1, "Expected 1 file packed");
        TEST_ASSERT(fs::exists(vaultFile), "Vault file was not created");
    }

    // Verify key
    {
        VaultReader reader(vaultFile.string());
        TEST_ASSERT(reader.verifyPassphrase(password), "Passphrase verification should succeed");
        TEST_ASSERT(!reader.verifyPassphrase("WrongPassword123"), "Wrong password must be rejected");
    }

    // Unpack
    fs::path outDir = testDir / "restored";
    {
        VaultReader reader(vaultFile.string());
        bool unlocked = reader.unlock(password);
        TEST_ASSERT(unlocked, "Unlocking with correct password should succeed");

        UnpackStats stats = reader.extractAll(outDir.string());
        TEST_ASSERT(stats.fileCount == 1, "Expected 1 file extracted");

        fs::path restoredFile = outDir / "hello.txt";
        TEST_ASSERT(fs::exists(restoredFile), "Restored file missing");

        auto restoredBytes = FileUtils::readFile(restoredFile);
        std::string restoredStr(reinterpret_cast<const char*>(restoredBytes.data()), restoredBytes.size());
        TEST_ASSERT(restoredStr == content, "Restored content does not match original");
    }

    fs::remove_all(testDir);
}

void test_multiple_files_and_directories() {
    fs::path testDir = "./temp_test_multi";
    fs::create_directories(testDir / "subfolder");
    fs::create_directories(testDir / "media");

    // File 1: Text file
    std::string t1 = "File 1 content in root of testDir";
    FileUtils::writeFile(testDir / "root.txt", reinterpret_cast<const uint8_t*>(t1.data()), t1.size());

    // File 2: Nested file
    std::string t2 = "File 2 nested in subfolder!";
    FileUtils::writeFile(testDir / "subfolder" / "nested.dat", reinterpret_cast<const uint8_t*>(t2.data()), t2.size());

    // File 3: Binary file with arbitrary bytes
    std::vector<uint8_t> binaryData(1024 * 64); // 64 KB
    for (size_t i = 0; i < binaryData.size(); ++i) {
        binaryData[i] = static_cast<uint8_t>(i & 0xFF);
    }
    FileUtils::writeFile(testDir / "media" / "binary.bin", binaryData);

    fs::path vaultFile = "./test_multi.vault";
    std::string password = "MultiFileKey!#$%^&*()";

    // Pack entire directory
    {
        VaultWriter writer(vaultFile.string(), password);
        writer.setKdfIterations(5000);
        writer.addSource(testDir.string());
        PackStats stats = writer.pack();
        TEST_ASSERT(stats.fileCount == 3, "Expected 3 files packed");
    }

    // Unpack to target
    fs::path extractDir = "./temp_test_multi_extracted";
    {
        VaultReader reader(vaultFile.string());
        TEST_ASSERT(reader.unlock(password), "Unlock failed for multi-file vault");
        TEST_ASSERT(reader.getEntries().size() == 3, "Catalog should have 3 entries");

        UnpackStats stats = reader.extractAll(extractDir.string());
        TEST_ASSERT(stats.fileCount == 3, "Expected 3 extracted files");

        // Verify binary exactness
        auto rootFound = FileUtils::readFile(extractDir / testDir.filename() / "root.txt");
        TEST_ASSERT(std::string(reinterpret_cast<char*>(rootFound.data()), rootFound.size()) == t1, "root.txt mismatch");

        auto nestedFound = FileUtils::readFile(extractDir / testDir.filename() / "subfolder" / "nested.dat");
        TEST_ASSERT(std::string(reinterpret_cast<char*>(nestedFound.data()), nestedFound.size()) == t2, "nested.dat mismatch");

        auto binFound = FileUtils::readFile(extractDir / testDir.filename() / "media" / "binary.bin");
        TEST_ASSERT(binFound == binaryData, "Binary file corrupted during roundtrip");
    }

    fs::remove_all(testDir);
    fs::remove_all(extractDir);
    fs::remove(vaultFile);
}

void test_tamper_detection() {
    fs::path testDir = "./temp_test_tamper";
    fs::create_directories(testDir);
    fs::path srcFile = testDir / "secret.txt";
    std::string content = "Cannot be tampered with!";
    FileUtils::writeFile(srcFile, reinterpret_cast<const uint8_t*>(content.data()), content.size());

    fs::path vaultFile = testDir / "tamper.vault";
    std::string password = "TamperProofPassword";

    {
        VaultWriter writer(vaultFile.string(), password);
        writer.setKdfIterations(5000);
        writer.addSource(srcFile.string());
        writer.pack();
    }

    // Tamper with one byte in the ciphertext
    {
        std::fstream f(vaultFile, std::ios::in | std::ios::out | std::ios::binary);
        f.seekp(sizeof(VaultHeader) + 5, std::ios::beg);
        char b = 0;
        f.read(&b, 1);
        b ^= 0x42; // flip bits
        f.seekp(sizeof(VaultHeader) + 5, std::ios::beg);
        f.write(&b, 1);
    }

    // Reading should strictly fail authentication
    {
        VaultReader reader(vaultFile.string());
        bool verified = reader.verifyPassphrase(password);
        TEST_ASSERT(!verified, "Tampered archive MUST fail verification");

        bool unlocked = reader.unlock(password);
        TEST_ASSERT(!unlocked, "Tampered archive MUST NOT unlock");
    }

    fs::remove_all(testDir);
}

void test_path_sanitization() {
    TEST_ASSERT(!FileUtils::isSafeRelativePath("../evil.txt"), "Relative parent path should be rejected");
    TEST_ASSERT(!FileUtils::isSafeRelativePath("foo/../../evil.txt"), "Nested relative parent path should be rejected");
    TEST_ASSERT(!FileUtils::isSafeRelativePath("/etc/passwd"), "Leading slash should be rejected");
    TEST_ASSERT(!FileUtils::isSafeRelativePath("C:\\Windows\\System32\\calc.exe"), "Drive letter should be rejected");
    TEST_ASSERT(FileUtils::isSafeRelativePath("documents/report.pdf"), "Clean relative path should be accepted");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "           SecureVault Test Suite Execution         \n";
    std::cout << "====================================================\n";

    RUN_TEST(test_sha256_known_vectors);
    RUN_TEST(test_hmac_sha256);
    RUN_TEST(test_aes256_cbc_roundtrip);
    RUN_TEST(test_path_sanitization);
    RUN_TEST(test_single_file_vault_pack_unpack);
    RUN_TEST(test_multiple_files_and_directories);
    RUN_TEST(test_tamper_detection);

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_passedTests << " Passed, " << g_failedTests << " Failed.\n";
    std::cout << "====================================================\n";

    return (g_failedTests == 0) ? 0 : 1;
}
