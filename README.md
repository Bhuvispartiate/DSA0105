# SecureVault

A modern, standalone, production-grade C++ secure file packaging and cryptographic archiving tool. **SecureVault** allows you to securely pack one or more files and entire folder trees into a single encrypted custom binary archive with the `.vault` file extension, and decrypt/extract them back using an authenticated key.

---

## Key Highlights

- **Custom `.vault` File Format**: High-performance binary specification featuring a structured header, authenticated metadata, and encrypted payload.
- **Enterprise-Grade Cryptography (Zero External Dependencies)**:
  - **AES-256 (CBC mode)**: 256-bit symmetric block cipher with standard PKCS#7 padding.
  - **HMAC-SHA256 (Encrypt-then-MAC)**: Constant-time authentication tag over the entire archive header and ciphertext, protecting against padding oracle attacks and bit-flipping tampering.
  - **PBKDF2-HMAC-SHA256**: Key derivation using 100,000 rounds and a 256-bit CSPRNG salt.
  - **Per-File SHA-256 Checksums**: Every file inside the vault is verified with a 32-byte cryptographic hash before being written to disk.
  - **CSPRNG**: Cryptographically secure pseudorandom byte generation (`BCrypt`/`CryptoAPI` on Windows, `/dev/urandom` on POSIX).
  - **Secure Zeroization**: Memory wiping of keys and plaintext buffers upon completion.
- **Multi-File & Directory Packaging**:
  - Handles single files, lists of files, or recursive directories preserving directory structures.
  - Path traversal and ZipSlip protection (strictly prevents relative escape via `..`).
- **Flexible Interface**:
  - Full CLI support (`pack`, `unpack`, `list`, `verify`, `info`).
  - Interactive menu mode when launched without command-line arguments.
  - Silent/hidden password masking in terminal sessions.
- **Automated Test Suite**: 100% automated test coverage testing NIST vectors, RFC vectors, roundtrip packaging, tampered archive rejection, and path sanitization.

---

## File Format Specification (`.vault`)

```
+------------------------------------------------------------------------+
|                          VAULT HEADER (104 bytes)                      |
+------------------------------------------------------------------------+
| Magic Bytes:          6 bytes ("SVAULT")                               |
| Format Version:       uint16_t (1)                                     |
| Cipher Algorithm ID:  uint16_t (1 = AES-256-CBC)                       |
| KDF Algorithm ID:     uint16_t (1 = PBKDF2-HMAC-SHA256)                |
| KDF Iterations:       uint32_t (default: 100,000 rounds)               |
| Random Salt:          32 bytes (CSPRNG generated)                      |
| Initialization Vector:16 bytes (CSPRNG generated)                      |
| Ciphertext Length:    uint64_t (Size of encrypted payload in bytes)    |
| HMAC-SHA256 Tag:      32 bytes (Computed over Header[0..71] + Payload) |
+------------------------------------------------------------------------+
|                        ENCRYPTED PAYLOAD (AES-256)                     |
+------------------------------------------------------------------------+
| Entry Count:          uint32_t                                         |
| For each Entry:                                                        |
|   - Path Length:      uint32_t                                         |
|   - Relative Path:    UTF-8 string (bytes)                             |
|   - Original Size:    uint64_t (bytes)                                 |
|   - Modified Time:    uint64_t (Unix seconds)                          |
|   - File SHA-256:     32 bytes                                         |
|   - File Data:        Raw byte stream (original file payload)          |
+------------------------------------------------------------------------+
```

---

## Project Structure

```
Cpp-Capstone/
├── CMakeLists.txt          # Modern CMake build configuration
├── Makefile                # Standard GNU / MinGW Makefile
├── build.bat               # Windows Batch build script
├── build.ps1               # PowerShell build script
├── README.md               # Complete documentation
├── include/
│   ├── Crypto.hpp          # AES-256, SHA-256, HMAC-SHA256, PBKDF2, CSPRNG
│   ├── FileUtils.hpp       # Safe filesystem operations & directory traversal
│   ├── VaultEntry.hpp      # File item data structure & metadata
│   ├── VaultFormat.hpp     # Binary header format layout and validation
│   ├── VaultReader.hpp     # Decryption, verification, listing & extraction
│   └── VaultWriter.hpp     # File packaging, key derivation & encryption
├── src/
│   ├── Crypto.cpp          # Cryptographic primitives implementation
│   ├── FileUtils.cpp       # Filesystem utilities & path sanitization
│   ├── VaultFormat.cpp     # Header serializer and validator
│   ├── VaultReader.cpp     # Vault parser and unpacker
│   ├── VaultWriter.cpp     # Vault packager and encryptor
│   └── main.cpp            # Production CLI and interactive terminal mode
└── tests/
    └── test_vault.cpp      # Automated unit and integration test runner
```

---

## Building the Project

### Method 1: PowerShell (Turnkey on Windows)
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

### Method 2: Windows Batch
```cmd
build.bat
```

### Method 3: CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Method 4: Makefile (MinGW / GCC / Clang)
```bash
make
```

---

## C++ Native Backend Server & Web UI

SecureVault includes an embedded, high-performance, multi-threaded C++ HTTP backend server ([`include/HttpServer.hpp`](include/HttpServer.hpp), [`src/HttpServer.cpp`](src/HttpServer.cpp)) with zero third-party dependencies (built directly on native sockets / Winsock).

The C++ server hosts the Web UI and executes all archival packaging, key derivation, AES-256 encryption, HMAC authentication, and extraction directly on the machine.

### Launching the Web UI
Double-click `vault.exe` or run it with no arguments to automatically start the C++ backend server (default port `8080`, with automatic fallback if busy) and open the Web UI in your default browser:

```powershell
.\vault.exe
```

You can also specify a custom port or start headless without opening a browser:

```powershell
.\vault.exe serve 8080
.\vault.exe serve 8080 --no-open
```

Output:
```
============================================================
      SecureVault - Authenticated C++ File Encryption       
           C++ Native Backend Server Listening              
============================================================

  Local URL: http://localhost:8080/
  Web Root:  .

Endpoints:
  GET  /              -> Web UI (index.html)
  GET  /api/status    -> Server & Crypto Core Status
  POST /api/pack      -> C++ Archival & Encryption Engine
  POST /api/unpack    -> C++ Decryption & Extraction Engine
  POST /api/verify    -> C++ HMAC Integrity Verifier
  POST /api/info      -> C++ Header Metadata Parser

Opening SecureVault UI in your default browser...
Keep this window open while using the UI. Press Ctrl+C to stop.
```

The Web UI automatically detects the C++ server and displays `C++ Backend: Online`. All encryption and decryption operations from the UI are routed directly to the C++ cryptographic engine.

### C++ REST API Endpoints

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/` | Serves `index.html` |
| `GET` | `/style.css` | Serves application stylesheet |
| `GET` | `/app.js` | Serves frontend client controller |
| `GET` | `/api/status` | Returns C++ backend status, version, and active cipher suites |
| `POST` | `/api/pack` | Accepts file list, key, and iterations; runs AES-256-CBC + HMAC-SHA256 in C++; returns `.vault` archive |
| `POST` | `/api/unpack` | Accepts `.vault` binary + key; verifies HMAC, decrypts in C++, verifies file SHA-256 digests; returns file catalog |
| `POST` | `/api/verify` | Authenticates archive integrity and key in constant time via C++ HMAC-SHA256 |
| `POST` | `/api/info` | Parses and returns the 104-byte unencrypted binary header envelope metadata |

---

## Command-Line Usage

### 1. Pack Files and Folders into a Vault
```bash
# Basic packaging (prompts securely for password without terminal echo)
vault.exe pack -o secure_backup.vault file1.txt docs/ photos/

# Specify password directly on CLI
vault.exe pack -o secure_backup.vault -k "StrongPassphrase123!" file1.txt src/

# Customize PBKDF2 iterations (e.g. 200,000 rounds for increased brute-force resistance)
vault.exe pack -o secure_backup.vault -k "Pass" --iterations 200000 notes.txt
```

### 2. Unpack and Extract Files
```bash
# Extract into default folder (./extracted)
vault.exe unpack -i secure_backup.vault

# Extract to a specified folder with key passed in
vault.exe unpack -i secure_backup.vault -o ./restored -k "StrongPassphrase123!"
```

### 3. List Vault Catalog (Without Extracting)
```bash
vault.exe list -i secure_backup.vault -k "StrongPassphrase123!"
```
Example Output:
```
Vault Catalog (3 entries):
---------------------------------------------------------------------------------------------------------
Relative Path                                   Size         Modified Date  SHA-256 (prefix)
---------------------------------------------------------------------------------------------------------
docs/financials.pdf                          1.20 MB   2026-10-29 14:22:00  4f2a89c091e3...
notes/passwords.txt                          2.40 KB   2026-10-30 08:10:15  b39f10a82ec1...
photos/id_scan.png                           4.15 MB   2026-10-28 19:45:30  7d81a91e004a...
---------------------------------------------------------------------------------------------------------
Total Archive Size: 5.35 MB across 3 files.
```

### 4. Verify Password & Archive Integrity
```bash
vault.exe verify -i secure_backup.vault -k "StrongPassphrase123!"
```
Output:
```
Computing HMAC-SHA256 over envelope and ciphertext...
[PASS] Authentication verified: Key is valid and archive integrity is intact.
```

### 5. Inspect Vault Envelope Metadata (No Password Required)
```bash
vault.exe info -i secure_backup.vault
```
Output:
```
Vault Envelope Metadata:
--------------------------------------------------
  Magic Signature:    SVAULT
  Format Version:     1
  Cipher Algorithm:   AES-256-CBC (ID 1)
  KDF Algorithm:      PBKDF2-HMAC-SHA256 (ID 1)
  PBKDF2 Iterations:  100000 rounds
  Random Salt (hex):  597c1431906085940761d928a2f37262d0eef454811d8c03760c1a7b3235ab41
  Random IV (hex):    8cb927304b33adb0709ecb2c38b521dc
  Ciphertext Size:    5.35 MB (5612304 bytes)
  HMAC-SHA256 Tag:    5cadddf32ad99fde8fb79aae98a2df35a9f83e4e7d9525b917bbb93f184fda58
--------------------------------------------------
```

### 6. Interactive Terminal CLI Mode
Run `vault.exe cli` to launch the menu-driven terminal interface:
```
============================================================
      SecureVault - Authenticated C++ File Encryption       
           AES-256-CBC | HMAC-SHA256 | PBKDF2               
============================================================

Choose an operation:
  1. Pack files/folders into a .vault archive (Encrypt)
  2. Unpack a .vault archive (Decrypt)
  3. List contents of a .vault archive
  4. Verify vault key & integrity
  5. Inspect vault envelope metadata
  6. Start C++ Native Backend Server (Web UI)
  7. Exit

Enter choice (1-7):
```

---

## Running the Automated Test Suite

Run the compiled test executable:
```powershell
.\test_vault.exe
```

Output:
```
====================================================
           SecureVault Test Suite Execution         
====================================================
[RUNNING] test_sha256_known_vectors... [PASS]
[RUNNING] test_hmac_sha256... [PASS]
[RUNNING] test_aes256_cbc_roundtrip... [PASS]
[RUNNING] test_path_sanitization... [PASS]
[RUNNING] test_single_file_vault_pack_unpack... [PASS]
[RUNNING] test_multiple_files_and_directories... [PASS]
[RUNNING] test_tamper_detection... [PASS]

====================================================
Test Summary: 7 Passed, 0 Failed.
====================================================
```

