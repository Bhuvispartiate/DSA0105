# SecureVault — C++ Capstone Project

**Student:** Bhuvanesh H  
**Register Number:** 192524027  
**Academic Year:** 2025–26  

A standalone C++17 authenticated file packaging, encryption, and archiving system with a custom `.vault` binary format, a native multi-threaded C++ HTTP backend server, and an interactive HTML/CSS/JS Web UI.

---

## Capstone Repository Structure

```
Capstone/
├── REVIEW 1.pptx              # Capstone Review 1 Presentation
├── REVIEW 2.pptx              # Capstone Review 2 Presentation
├── README.md                  # Project Documentation
└── Implementation/            # Complete SecureVault C++ Source & Web UI
    ├── CMakeLists.txt         # CMake build configuration
    ├── Makefile               # GNU / MinGW Makefile
    ├── build.bat              # Windows Batch build script
    ├── build.ps1              # PowerShell build script
    ├── PROJECT_OVERVIEW.md    # Architectural & 4-module presentation overview
    ├── index.html             # Web UI layout
    ├── style.css              # Web UI dark theme stylesheet
    ├── app.js                 # Web UI client & C++ REST connector
    ├── include/
    │   ├── Base64.hpp         # Base64 codec for binary transport
    │   ├── Crypto.hpp         # AES-256, SHA-256, HMAC-SHA256, PBKDF2, CSPRNG
    │   ├── FileUtils.hpp      # Safe filesystem operations & directory traversal
    │   ├── HttpServer.hpp     # Native C++ HTTP/1.1 REST server
    │   ├── Json.hpp           # Header-only JSON parser & serializer
    │   ├── VaultEntry.hpp     # File entry metadata structure
    │   ├── VaultFormat.hpp    # 104-byte binary header format & validation
    │   ├── VaultReader.hpp    # Authentication, decryption & extraction
    │   └── VaultWriter.hpp    # Archival packaging, key derivation & encryption
    ├── src/
    │   ├── Crypto.cpp         # Cryptographic primitives implementation
    │   ├── FileUtils.cpp      # Path sanitization & binary I/O
    │   ├── HttpServer.cpp     # Multi-threaded socket server & REST routes
    │   ├── VaultFormat.cpp    # Header serializer & validator
    │   ├── VaultReader.cpp    # Vault parser & unpacker
    │   ├── VaultWriter.cpp    # Vault packager & encryptor
    │   └── main.cpp           # Application entry point (UI server + CLI)
    └── tests/
        └── test_vault.cpp     # Automated unit & security test suite
```

---

## Key Highlights

- **Custom `.vault` Binary Format**: 104-byte authenticated outer envelope header paired with an AES-256 encrypted manifest and file payload.
- **Zero-Dependency Standard Cryptography**:
  - **AES-256-CBC (FIPS 197)**: 256-bit symmetric encryption with PKCS#7 padding.
  - **HMAC-SHA256 (RFC 2104)**: Encrypt-then-MAC authentication over header + ciphertext using constant-time comparison to neutralize tampering and padding oracle attacks.
  - **PBKDF2-HMAC-SHA256 (RFC 8018)**: 100,000-round key derivation with a 256-bit CSPRNG salt, deriving separate 256-bit encryption and authentication keys.
  - **Per-File SHA-256 Digests (FIPS 180-4)**: Every archived file is verified bit-exact prior to extraction.
  - **Secure Memory Zeroization**: Keys, round schedules, and plaintext buffers are wiped immediately after use.
- **Native C++ Backend + Web UI**: Double-clicking `vault.exe` launches the embedded C++ Winsock HTTP server and opens the interactive Web UI automatically.
- **Full CLI Support**: Supports `pack`, `unpack`, `list`, `verify`, `info`, `serve`, and interactive terminal mode (`cli`).
- **Path Traversal Protection**: Strict relative path validation blocks ZipSlip (`..`) and absolute path escapes.

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

## Building the Project

Navigate into the `Implementation` directory first:

```powershell
cd Capstone\Implementation
```

### Method 1: PowerShell (Windows)
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

## Running SecureVault

### 1. Launch Web UI with C++ Backend (Default)
Double-click `vault.exe` or run it with no arguments to start the C++ native HTTP server (default port `8080`, auto-fallback if busy) and open the Web UI automatically:

```powershell
.\vault.exe
```

Optional server flags:
```powershell
.\vault.exe serve 8080
.\vault.exe serve 8080 --no-open
```

### C++ REST API Endpoints

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/` | Serves `index.html` |
| `GET` | `/style.css` | Serves application stylesheet |
| `GET` | `/app.js` | Serves frontend client controller |
| `GET` | `/api/status` | Returns C++ backend status, active port, and cipher suite |
| `POST` | `/api/pack` | Packages and encrypts files in C++, returning the `.vault` archive |
| `POST` | `/api/unpack` | Authenticates HMAC, decrypts in C++, verifies SHA-256 digests, returns files |
| `POST` | `/api/verify` | Constant-time HMAC key and integrity verification |
| `POST` | `/api/info` | Parses the 104-byte unencrypted binary header envelope |

### 2. Command-Line Usage

```powershell
# Pack files and directories into a .vault archive
.\vault.exe pack -o backup.vault -k "StrongPassphrase123!" file1.txt docs/

# Unpack and restore files from a .vault archive
.\vault.exe unpack -i backup.vault -o ./restored -k "StrongPassphrase123!"

# List catalog entries without extracting
.\vault.exe list -i backup.vault -k "StrongPassphrase123!"

# Verify key & archive integrity
.\vault.exe verify -i backup.vault -k "StrongPassphrase123!"

# Inspect header envelope metadata (no key required)
.\vault.exe info -i backup.vault

# Interactive terminal menu
.\vault.exe cli
```

---

## Automated Test Suite

Run the compiled test binary inside `Capstone\Implementation`:

```powershell
.\test_vault.exe
```

Verified tests:
- `test_sha256_known_vectors` (NIST FIPS 180-4 vectors)
- `test_hmac_sha256` (RFC 4231 Case 2 vector)
- `test_aes256_cbc_roundtrip`
- `test_path_sanitization` (ZipSlip defense)
- `test_single_file_vault_pack_unpack`
- `test_multiple_files_and_directories`
- `test_tamper_detection` (1-bit ciphertext modification rejection)
