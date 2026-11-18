# SecureVault: System Architecture and Technical Implementation

A technical reference guide detailing the concepts, architecture, module divisions, and cryptographic implementations of the SecureVault authenticated archiving system.

---

## 1. Project Concept and Objectives

SecureVault is an authenticated multi-file cryptographic archiving system designed to package arbitrary files and directory hierarchies into a single unified binary archive with the `.vault` format extension.

### Core Objectives
- **Confidentiality**: Protect all file contents, file names, folder structures, timestamps, and catalog manifests using symmetric AES-256 encryption.
- **Authenticity and Integrity**: Implement an Encrypt-then-MAC (EtM) architecture using HMAC-SHA256 to guarantee that neither the archive header nor the encrypted payload can be modified without immediate detection.
- **Zero Third-Party Cryptographic Dependencies**: All cryptographic primitives (AES-256, HMAC-SHA256, PBKDF2, SHA-256) are implemented directly against NIST and IETF specifications, eliminating supply-chain attack vectors.
- **Dual Interface Architecture**: Provides both a native command-line interface and a high-performance native HTTP backend server powering a clean web interface.

---

## 2. Four-Module Team Presentation Structure

The system is partitioned into four balanced engineering modules, allowing a team of four to present the architecture comprehensively:

```
+-----------------------------------------------------------------------------------+
|                            SECUREVAULT SYSTEM MODULES                             |
+-----------------------------------------------------------------------------------+
| Module 1: Cryptographic Engine        | Module 2: Binary Archival Storage         |
| Focus: AES-256, PBKDF2, HMAC, CSPRNG  | Focus: Envelope Layout, Catalog Packing   |
+---------------------------------------+-------------------------------------------+
| Module 3: Native Backend HTTP Engine  | Module 4: Client Interface and QA Suite   |
| Focus: Multi-threaded Sockets, REST   | Focus: Responsive UI, Security Benchmarks |
+-----------------------------------------------------------------------------------+
```

### Module 1: Core Cryptographic Engine
- **Presenter Focus**: Mathematical and algorithmic foundations of the security primitives.
- **Key Responsibilities**:
  - Implementation of the AES-256 block cipher adhering to NIST FIPS 197.
  - PBKDF2 key derivation adhering to RFC 8018 with 100,000 iterations.
  - Message Authentication Code calculation adhering to RFC 2104.
  - Constant-time signature verification routines preventing timing side-channel attacks.
  - System-level CSPRNG entropy harvesting and volatile memory zeroization.

### Module 2: Binary Archival Storage Engine
- **Presenter Focus**: Data structures, binary layouts, serialization, and filesystem safety.
- **Key Responsibilities**:
  - Design of the 104-byte unencrypted outer envelope header.
  - Serialization of the encrypted catalog manifest containing file relative paths, original file sizes, Unix timestamps, and individual file hashes.
  - Recursive directory tree exploration and path normalization.
  - Defensive path parsing to prevent directory traversal and ZipSlip vulnerabilities.

### Module 3: Native Backend HTTP Server and REST API
- **Presenter Focus**: Network communications, concurrency, serialization, and endpoint routing.
- **Key Responsibilities**:
  - Non-blocking socket communication using native operating system socket APIs.
  - Multi-threaded connection pooling spawning independent worker routines per request.
  - Custom JSON recursive descent parser and serializer without external dependencies.
  - High-throughput Base64 codec for binary file streams.
  - REST API routing for packing, unpacking, verification, and metadata inspection.

### Module 4: Client Interface and Security Verification Suite
- **Presenter Focus**: User experience, operational workflow, automated testing, and threat resistance.
- **Key Responsibilities**:
  - Responsive, dark-themed user interface without emojis or visual clutter.
  - Client-side bulk download bundling via standard ZIP generation.
  - Automated test runner verifying NIST standard vectors for SHA-256 and RFC test vectors for HMAC.
  - Regression testing demonstrating immediate detection and rejection of 1-bit corrupted archives.

---

## 3. Technology Stack and Specifications

| Category | Technology / Standard | Technical Role |
| :--- | :--- | :--- |
| Core Language | Modern C++ (C++17) | System architecture, cryptographic engine, server runtime |
| Symmetric Cipher | AES-256-CBC (FIPS 197) | 256-bit symmetric block encryption with PKCS#7 padding |
| Key Derivation | PBKDF2-HMAC-SHA256 (RFC 8018) | Slow key derivation with 100,000 rounds and 32-byte salt |
| Message Authentication | HMAC-SHA256 (RFC 2104) | Cryptographic signature over header and ciphertext |
| Cryptographic Hash | SHA-256 (FIPS 180-4) | Per-file integrity checks and HMAC digest routines |
| Operating System Entropy| Win32 CryptoAPI / POSIX urandom | Cryptographically secure pseudo-random number generation |
| Memory Protection | OS Secure Zero Memory routines | Volatile memory scrubbing of keys and plaintext buffers |
| Networking | Native Sockets / Winsock 2.2 | Multi-threaded HTTP/1.1 REST backend server |
| Data Serialization | Custom JSON (RFC 8259) | Lightweight REST API request and response messaging |
| Binary Encoding | Base64 (RFC 4648) | Binary payload transmission across HTTP endpoints |
| Frontend Layer | HTML5, Modern CSS3, ES2022 | Interactive web client interface |

---

## 4. Binary Envelope Specification (`.vault`)

The `.vault` format separates archive metadata into two zones: an outer 104-byte authenticated envelope header, and an inner AES-256 encrypted payload.

```
+------------------------------------------------------------------------+
|                     OUTER ENVELOPE HEADER (104 bytes)                  |
+------------------------------------------------------------------------+
| Offset  | Size (Bytes) | Field Name         | Description              |
|---------+--------------+--------------------+--------------------------|
| 0x00    | 6            | Magic Bytes        | "SVAULT" ASCII identifier|
| 0x06    | 2            | Format Version     | uint16 (Value: 1)        |
| 0x08    | 2            | Cipher ID          | uint16 (1 = AES-256-CBC) |
| 0x0A    | 2            | KDF ID             | uint16 (1 = PBKDF2-HMAC) |
| 0x0C    | 4            | KDF Iterations     | uint32 (Default: 100,000)|
| 0x10    | 32           | Cryptographic Salt | 256 bits CSPRNG entropy  |
| 0x30    | 16           | Init Vector (IV)   | 128 bits CSPRNG entropy  |
| 0x40    | 8            | Ciphertext Length  | uint64 payload byte size |
| 0x48    | 32           | HMAC-SHA256 Tag    | Signature over 0x00-0x47 |
+------------------------------------------------------------------------+
|                       ENCRYPTED PAYLOAD ZONE                           |
+------------------------------------------------------------------------+
| Entry Count: uint32                                                    |
| For each archived file:                                                |
|   - Path Length:      uint32                                           |
|   - Relative Path:    UTF-8 String bytes                               |
|   - Original Size:    uint64                                           |
|   - Modified Time:    uint64 (Unix epoch seconds)                      |
|   - File SHA-256:     32 bytes                                         |
|   - File Payload:     Raw bytes (Length equal to Original Size)        |
+------------------------------------------------------------------------+
```

---

## 5. Cryptographic Implementation Details

### Key Derivation Mechanics (PBKDF2-HMAC-SHA256)
A single password must never be reused for both encryption and authentication. SecureVault applies the key separation principle:
1. The user's passphrase, a 32-byte cryptographically secure random salt, and 100,000 iterations are supplied to PBKDF2.
2. The Pseudorandom Function (PRF) evaluates HMAC-SHA256 across blocks:
   $$U_1 = \text{PRF}(\text{Password}, \text{Salt} \parallel \text{BlockIndex})$$
   $$U_c = \text{PRF}(\text{Password}, U_{c-1})$$
   $$F = U_1 \oplus U_2 \oplus \dots \oplus U_{100000}$$
3. The derived 512-bit master key is partitioned:
   - **Bytes 0 to 31**: AES-256 Symmetric Encryption Key.
   - **Bytes 32 to 63**: HMAC-SHA256 Authentication Key.

### Symmetric Encryption Mechanics (AES-256-CBC)
- **Key Expansion**: Generates 15 round keys (60 32-bit words) from the 256-bit AES key.
- **State Transformations**: Each round applies `SubBytes`, `ShiftRows`, `MixColumns` over $GF(2^8)$, and `AddRoundKey`. Round 14 omits `MixColumns`.
- **Chaining**: Cipher Block Chaining mode XORs each 16-byte plaintext block with the preceding ciphertext block (the first block XORs with the random IV).
- **Padding**: Standard PKCS#7 appends $N$ bytes of value $N$ (where $1 \le N \le 16$) to ensure the payload is an exact multiple of 16 bytes.

### Authenticated Encryption (Encrypt-then-MAC)
To guarantee security against chosen-ciphertext and padding oracle attacks, authentication is computed using the Encrypt-then-MAC paradigm:
1. Ciphertext is produced by encrypting the serialized plaintext payload.
2. The first 72 bytes of the outer envelope header (magic, version, cipher ID, KDF ID, iteration count, salt, IV, ciphertext length) are concatenated with the complete ciphertext buffer.
3. An HMAC-SHA256 signature is calculated over this combined buffer using the dedicated authentication key.
4. The resulting 32-byte tag is stored at byte offset 72 of the header.

---

## 6. Execution Pipelines

### Encryption and Archiving Workflow
```
[User Input Files]
       |
       v
1. Calculate individual file SHA-256 hashes and gather metadata
       |
       v
2. Serialize manifest catalog and file data into contiguous byte buffer
       |
       v
3. Generate 32-byte Salt and 16-byte IV via OS CSPRNG
       |
       v
4. Derive 64-byte master key via PBKDF2 (100,000 iterations)
   |--> Extract 32-byte AES Key
   |--> Extract 32-byte HMAC Key
       |
       v
5. Encrypt serialized buffer via AES-256-CBC with PKCS#7 padding
   (Wipe unencrypted plaintext buffer immediately)
       |
       v
6. Assemble 104-byte header and compute HMAC-SHA256 over Header[0..71] + Ciphertext
   (Wipe derived keys from memory)
       |
       v
7. Write Header + Ciphertext to disk as .vault archive
```

### Decryption and Extraction Workflow
```
[.vault Archive File]
       |
       v
1. Read and validate 104-byte header envelope (Magic, Version, Sizes)
       |
       v
2. Derive 64-byte master key via PBKDF2 using Header.Salt and Header.Iterations
   |--> Extract 32-byte AES Key
   |--> Extract 32-byte HMAC Key
       |
       v
3. Constant-Time Authentication Check:
   Compute HMAC-SHA256(Header[0..71] + Ciphertext) == Header.HMAC_Tag
       |
       +--> MISMATCH: Halt immediately. Zeroize memory. Return authentication error.
       |              (No decryption attempted; prevents oracle attacks)
       |
       +--> MATCH: Authentication verified. Proceed to Step 4.
       |
       v
4. Decrypt payload via AES-256-CBC and strip PKCS#7 padding
   (Wipe derived keys immediately)
       |
       v
5. Deserialize catalog manifest (entry count, relative paths, file sizes, timestamps)
       |
       v
6. For each file entry:
   - Validate relative path safety (Reject '../' and absolute root paths)
   - Verify SHA-256(DecryptedBytes) == Recorded SHA-256 Checksum
       |
       v
7. Write verified files to target destination on disk
```

---

## 7. Security Mitigations and Defenses

1. **Padding Oracle Immunity**: Because verification occurs in constant time before decryption, any tampered byte or incorrect password triggers an immediate authentication rejection. Decryption routines are never executed on unauthenticated ciphertext.
2. **Timing Attack Protection**: HMAC tag validation uses bitwise accumulation across all 32 bytes rather than early-exit byte comparison, ensuring execution duration is identical regardless of where a mismatch occurs.
3. **Rainbow Table Invalidation**: Every `.vault` archive generates an independent 256-bit salt from the operating system's cryptographic random pool, forcing adversaries to compute $100,000$ PBKDF2 rounds per archive during brute-force attempts.
4. **Directory Traversal (ZipSlip) Prevention**: Every file path stored in the catalog is sanitized during extraction. Any path attempting to reference parent folders (`../`) or drive letters is rejected, preventing malicious archives from overwriting system files.
5. **Memory Residue Scrubbing**: All intermediate cryptographic structures, expanded key schedules, and plaintext file buffers are actively wiped from memory using operating system zeroization routines immediately after use.

