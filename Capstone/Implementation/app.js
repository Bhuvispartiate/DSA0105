// SecureVault - Cryptographic Client & C++ Backend Connector
// Zero Emojis • Zero AI Slop • Native C++ Backend Integration

const VAULT_MAGIC = new Uint8Array([0x53, 0x56, 0x41, 0x55, 0x4c, 0x54]); // "SVAULT"
const HEADER_SIZE = 104;
const PRE_HMAC_SIZE = 72;

let isCppBackendOnline = false;
let apiBaseUrl = '';

// -----------------------------------------------------------------------------
// Logging & Toast Utilities
// -----------------------------------------------------------------------------
function addLog(logElementId, type, message) {
  const container = document.getElementById(logElementId);
  if (!container) return;

  const now = new Date();
  const timeStr = `[${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}:${String(now.getSeconds()).padStart(2, '0')}]`;

  const entry = document.createElement('div');
  entry.className = `log-entry ${type}`;
  entry.innerHTML = `<span class="log-time">${timeStr}</span> <span>${escapeHtml(message)}</span>`;
  container.appendChild(entry);
  container.scrollTop = container.scrollHeight;
}

function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  if (!container) return;

  const toast = document.createElement('div');
  toast.className = `toast ${type}`;
  toast.textContent = message;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transition = 'opacity 0.3s ease';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}

function formatBytes(bytes) {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

function toHex(uint8Array) {
  return Array.from(uint8Array)
    .map(b => b.toString(16).padStart(2, '0'))
    .join('');
}

function uint8ToBase64(bytes) {
  const chunkSize = 0x8000;
  const parts = [];
  for (let i = 0; i < bytes.byteLength; i += chunkSize) {
    parts.push(String.fromCharCode.apply(null, bytes.subarray(i, i + chunkSize)));
  }
  return window.btoa(parts.join(''));
}

function base64ToUint8(base64) {
  const binary = window.atob(base64);
  const bytes = new Uint8Array(binary.length);
  for (let i = 0; i < binary.length; i++) {
    bytes[i] = binary.charCodeAt(i);
  }
  return bytes;
}

// -----------------------------------------------------------------------------
// Check C++ Backend Health Status
// -----------------------------------------------------------------------------
async function checkBackendStatus() {
  const dot = document.getElementById('backend-dot');
  const label = document.getElementById('backend-label');

  const candidates = window.location.protocol === 'file:'
    ? ['http://localhost:8080', 'http://localhost:8081']
    : ['', 'http://localhost:8080'];

  for (const base of candidates) {
    try {
      const res = await fetch(`${base}/api/status`, { method: 'GET', cache: 'no-store' });
      if (res.ok) {
        const data = await res.json();
        if (data.status === 'online') {
          isCppBackendOnline = true;
          apiBaseUrl = base;
          const activePort = data.port || window.location.port || '8080';
          dot.style.backgroundColor = 'var(--accent-success)';
          label.textContent = `C++ Backend: Online (:${activePort})`;
          addLog('pack-log', 'success', `Connected to C++ Native Engine (${data.backend} v${data.version}) on port ${activePort}.`);
          addLog('unpack-log', 'success', `C++ Backend Server Active on :${activePort}. Ready to process vaults.`);
          return;
        }
      }
    } catch (e) {
      // Try next candidate
    }
  }

  isCppBackendOnline = false;
  dot.style.backgroundColor = 'var(--accent-warning)';
  label.textContent = 'C++ Backend: Offline (Standalone)';
  addLog('pack-log', 'warning', 'C++ Backend offline. Launch vault.exe to connect to the C++ native server.');
}

// -----------------------------------------------------------------------------
// Pure JS Lightweight ZIP Builder (For Download All)
// -----------------------------------------------------------------------------
const CRC_TABLE = new Uint32Array(256);
for (let n = 0; n < 256; n++) {
  let c = n;
  for (let k = 0; k < 8; k++) {
    c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1);
  }
  CRC_TABLE[n] = c >>> 0;
}

function computeCRC32(bytes) {
  let crc = 0xffffffff;
  for (let i = 0; i < bytes.length; i++) {
    crc = CRC_TABLE[(crc ^ bytes[i]) & 0xff] ^ (crc >>> 8);
  }
  return (crc ^ 0xffffffff) >>> 0;
}

function createZipBlob(fileEntries) {
  const parts = [];
  const centralDir = [];
  let offset = 0;
  const enc = new TextEncoder();

  for (const entry of fileEntries) {
    const filenameBytes = enc.encode(entry.relativePath);
    const crc = computeCRC32(entry.data);
    const size = entry.data.length;

    // Local file header (30 bytes + filename)
    const localHeader = new Uint8Array(30 + filenameBytes.length);
    const v = new DataView(localHeader.buffer);
    v.setUint32(0, 0x04034b50, true);
    v.setUint16(4, 20, true);
    v.setUint16(6, 0, true);
    v.setUint16(8, 0, true);  // store
    v.setUint16(10, 0, true);
    v.setUint16(12, 0, true);
    v.setUint32(14, crc, true);
    v.setUint32(18, size, true);
    v.setUint32(22, size, true);
    v.setUint16(26, filenameBytes.length, true);
    v.setUint16(28, 0, true);
    localHeader.set(filenameBytes, 30);

    parts.push(localHeader);
    parts.push(entry.data);

    // Central directory header (46 bytes + filename)
    const cdHeader = new Uint8Array(46 + filenameBytes.length);
    const cdv = new DataView(cdHeader.buffer);
    cdv.setUint32(0, 0x02014b50, true);
    cdv.setUint16(4, 20, true);
    cdv.setUint16(6, 20, true);
    cdv.setUint16(8, 0, true);
    cdv.setUint16(10, 0, true);
    cdv.setUint16(12, 0, true);
    cdv.setUint16(14, 0, true);
    cdv.setUint32(16, crc, true);
    cdv.setUint32(20, size, true);
    cdv.setUint32(24, size, true);
    cdv.setUint16(28, filenameBytes.length, true);
    cdv.setUint16(30, 0, true);
    cdv.setUint16(32, 0, true);
    cdv.setUint16(34, 0, true);
    cdv.setUint16(36, 0, true);
    cdv.setUint32(38, 0, true);
    cdv.setUint32(42, offset, true);
    cdHeader.set(filenameBytes, 46);

    centralDir.push(cdHeader);
    offset += localHeader.length + entry.data.length;
  }

  const cdOffset = offset;
  let cdSize = 0;
  for (const h of centralDir) {
    parts.push(h);
    cdSize += h.length;
  }

  // End of central directory record (22 bytes)
  const eocd = new Uint8Array(22);
  const ev = new DataView(eocd.buffer);
  ev.setUint32(0, 0x06054b50, true);
  ev.setUint16(4, 0, true);
  ev.setUint16(6, 0, true);
  ev.setUint16(8, fileEntries.length, true);
  ev.setUint16(10, fileEntries.length, true);
  ev.setUint32(12, cdSize, true);
  ev.setUint32(16, cdOffset, true);
  ev.setUint16(20, 0, true);

  parts.push(eocd);
  return new Blob(parts, { type: 'application/zip' });
}

// -----------------------------------------------------------------------------
// Tab 1: Pack & Encrypt Operations
// -----------------------------------------------------------------------------
const stagedFiles = [];

const packDropZone = document.getElementById('pack-drop-zone');
const packFileInput = document.getElementById('pack-file-input');
const packStagedWrapper = document.getElementById('pack-staged-wrapper');
const packStagedTbody = document.getElementById('pack-staged-tbody');
const packSummaryText = document.getElementById('pack-summary-text');
const packClearAll = document.getElementById('pack-clear-all');
const packExecuteBtn = document.getElementById('pack-execute-btn');

function updateStagedUI() {
  if (stagedFiles.length === 0) {
    packStagedWrapper.style.display = 'none';
    packDropZone.style.display = 'flex';
    return;
  }

  packDropZone.style.display = 'flex';
  packStagedWrapper.style.display = 'block';
  packStagedTbody.innerHTML = '';

  let totalBytes = 0;
  stagedFiles.forEach((file, idx) => {
    totalBytes += file.data.length;
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td style="font-weight: 500;">${escapeHtml(file.name)}</td>
      <td class="font-mono text-muted">${escapeHtml(file.relativePath)}</td>
      <td class="text-right font-mono">${formatBytes(file.data.length)}</td>
      <td class="text-right">
        <button class="btn-icon-only remove-file-btn" data-index="${idx}" title="Remove file">
          <svg style="width:14px;height:14px;" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></line></svg>
        </button>
      </td>
    `;
    packStagedTbody.appendChild(tr);
  });

  packSummaryText.textContent = `${stagedFiles.length} file(s) staged • Total size: ${formatBytes(totalBytes)}`;

  document.querySelectorAll('.remove-file-btn').forEach(btn => {
    btn.onclick = () => {
      const i = parseInt(btn.dataset.index, 10);
      stagedFiles.splice(i, 1);
      updateStagedUI();
    };
  });
}

packClearAll.onclick = () => {
  stagedFiles.length = 0;
  updateStagedUI();
  addLog('pack-log', 'info', 'Cleared all staged files.');
};

async function handleAddedFiles(fileList) {
  for (const file of fileList) {
    const buffer = await file.arrayBuffer();
    const cleanPath = (file.webkitRelativePath || file.name).replace(/^[\/\\]+/, '');
    stagedFiles.push({
      name: file.name,
      relativePath: cleanPath,
      modifiedTime: Math.floor(file.lastModified / 1000),
      data: new Uint8Array(buffer)
    });
    addLog('pack-log', 'info', `Staged: ${cleanPath} (${formatBytes(file.size)})`);
  }
  updateStagedUI();
}

packFileInput.onchange = (e) => {
  if (e.target.files) handleAddedFiles(e.target.files);
  e.target.value = '';
};

packDropZone.ondragover = (e) => {
  e.preventDefault();
  packDropZone.classList.add('dragover');
};
packDropZone.ondragleave = () => packDropZone.classList.remove('dragover');
packDropZone.ondrop = (e) => {
  e.preventDefault();
  packDropZone.classList.remove('dragover');
  if (e.dataTransfer.files) handleAddedFiles(e.dataTransfer.files);
};

packExecuteBtn.onclick = async () => {
  if (stagedFiles.length === 0) {
    showToast('Please stage at least one file to encrypt.', 'error');
    return;
  }

  const key = document.getElementById('pack-key').value;
  const confirmKey = document.getElementById('pack-confirm-key').value;
  const iterations = parseInt(document.getElementById('pack-iterations').value, 10) || 100000;
  let vaultName = document.getElementById('pack-vault-name').value.trim() || 'archive.vault';
  if (!vaultName.endsWith('.vault')) vaultName += '.vault';

  if (!key) {
    showToast('Passphrase cannot be empty.', 'error');
    return;
  }
  if (key !== confirmKey) {
    showToast('Passphrases do not match.', 'error');
    return;
  }
  if (iterations < 1000) {
    showToast('PBKDF2 iterations must be at least 1,000.', 'error');
    return;
  }

  packExecuteBtn.disabled = true;

  if (isCppBackendOnline) {
    addLog('pack-log', 'info', `[C++ BACKEND] Forwarding request to C++ Native Engine (POST /api/pack)...`);
    try {
      const payloadFiles = stagedFiles.map(f => ({
        path: f.relativePath,
        data: uint8ToBase64(f.data),
        mtime: f.modifiedTime
      }));

      const reqBody = {
        passphrase: key,
        iterations: iterations,
        files: payloadFiles
      };

      const res = await fetch(`${apiBaseUrl}/api/pack`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(reqBody)
      });

      const data = await res.json();
      if (!res.ok || !data.success) {
        throw new Error(data.error || 'Server packing error');
      }

      addLog('pack-log', 'success', `[C++ BACKEND] Packed ${data.fileCount} file(s) in ${data.durationMs.toFixed(1)} ms. Total vault size: ${formatBytes(data.vaultBytes)}.`);

      const vaultBytes = base64ToUint8(data.vaultData);
      const blob = new Blob([vaultBytes], { type: 'application/octet-stream' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = vaultName;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);

      showToast(`C++ Vault created and downloaded: ${vaultName}`, 'success');
    } catch (err) {
      addLog('pack-log', 'error', `[C++ BACKEND] Error: ${err.message}`);
      showToast(err.message, 'error');
    } finally {
      packExecuteBtn.disabled = false;
    }
  } else {
    // Standalone Web Crypto fallback
    addLog('pack-log', 'info', `[STANDALONE] Starting Web Crypto fallback encryption (${iterations.toLocaleString()} rounds)...`);
    try {
      const startTime = performance.now();
      const plaintext = await serializePayloadFallback(stagedFiles);
      const salt = new Uint8Array(32);
      crypto.getRandomValues(salt);
      const iv = new Uint8Array(16);
      crypto.getRandomValues(iv);

      const { aesBytes, hmacBytes } = await deriveKeysFallback(key, salt, iterations);
      const aesCryptoKey = await crypto.subtle.importKey('raw', aesBytes, { name: 'AES-CBC' }, false, ['encrypt']);
      const ciphertext = new Uint8Array(await crypto.subtle.encrypt({ name: 'AES-CBC', iv }, aesCryptoKey, plaintext));

      const header = new Uint8Array(HEADER_SIZE);
      const headerView = new DataView(header.buffer);
      header.set(VAULT_MAGIC, 0);
      headerView.setUint16(6, 1, true);
      headerView.setUint16(8, 1, true);
      headerView.setUint16(10, 1, true);
      headerView.setUint32(12, iterations, true);
      header.set(salt, 16);
      header.set(iv, 48);
      headerView.setBigUint64(64, BigInt(ciphertext.length), true);

      const hmacData = new Uint8Array(PRE_HMAC_SIZE + ciphertext.length);
      hmacData.set(header.subarray(0, PRE_HMAC_SIZE), 0);
      hmacData.set(ciphertext, PRE_HMAC_SIZE);

      const hmacCryptoKey = await crypto.subtle.importKey('raw', hmacBytes, { name: 'HMAC', hash: 'SHA-256' }, false, ['sign']);
      const hmacSig = new Uint8Array(await crypto.subtle.sign('HMAC', hmacCryptoKey, hmacData));
      header.set(hmacSig, 72);

      const finalVault = new Uint8Array(HEADER_SIZE + ciphertext.length);
      finalVault.set(header, 0);
      finalVault.set(ciphertext, HEADER_SIZE);

      const duration = (performance.now() - startTime).toFixed(1);
      addLog('pack-log', 'success', `Vault archive created in ${duration} ms (${formatBytes(finalVault.length)}).`);

      const blob = new Blob([finalVault], { type: 'application/octet-stream' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = vaultName;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      showToast(`Vault downloaded: ${vaultName}`, 'success');
    } catch (err) {
      addLog('pack-log', 'error', `Encryption failure: ${err.message}`);
      showToast(`Error: ${err.message}`, 'error');
    } finally {
      packExecuteBtn.disabled = false;
    }
  }
};

// -----------------------------------------------------------------------------
// Tab 2: Unpack & Decrypt Operations
// -----------------------------------------------------------------------------
let selectedVaultBytes = null;
let currentDecryptedEntries = [];

const unpackDropZone = document.getElementById('unpack-drop-zone');
const unpackFileInput = document.getElementById('unpack-file-input');
const unpackFilePreview = document.getElementById('unpack-file-preview');
const unpackDetectedTitle = document.getElementById('unpack-detected-title');
const unpackDetectedMeta = document.getElementById('unpack-detected-meta');
const unpackExecuteBtn = document.getElementById('unpack-execute-btn');
const unpackResultsCard = document.getElementById('unpack-results-card');
const unpackCatalogTbody = document.getElementById('unpack-catalog-tbody');
const unpackCountBadge = document.getElementById('unpack-count-badge');
const unpackDownloadAll = document.getElementById('unpack-download-all');

function parseVaultHeaderLocal(bytes) {
  if (bytes.length < HEADER_SIZE) throw new Error("File smaller than 104-byte header.");
  const magic = bytes.subarray(0, 6);
  for (let i = 0; i < 6; i++) {
    if (magic[i] !== VAULT_MAGIC[i]) throw new Error("Invalid magic bytes: Not a SecureVault archive.");
  }
  const view = new DataView(bytes.buffer, bytes.byteOffset, HEADER_SIZE);
  const version = view.getUint16(6, true);
  const cipherId = view.getUint16(8, true);
  const kdfId = view.getUint16(10, true);
  const iterations = view.getUint32(12, true);
  const salt = bytes.subarray(16, 48);
  const iv = bytes.subarray(48, 64);
  const ciphertextLength = Number(view.getBigUint64(64, true));
  const expectedHmac = bytes.subarray(72, 104);
  return { version, cipherId, kdfId, iterations, salt, iv, ciphertextLength, expectedHmac };
}

function handleVaultFileSelect(file) {
  const reader = new FileReader();
  reader.onload = () => {
    try {
      selectedVaultBytes = new Uint8Array(reader.result);
      const h = parseVaultHeaderLocal(selectedVaultBytes);

      unpackFilePreview.style.display = 'flex';
      unpackDetectedTitle.textContent = `${file.name} (${formatBytes(file.size)})`;
      unpackDetectedMeta.textContent = `Version ${h.version} • AES-256-CBC • PBKDF2 (${h.iterations.toLocaleString()} rounds) • Salt: ${toHex(h.salt.subarray(0, 8))}...`;

      addLog('unpack-log', 'info', `Vault file loaded: ${file.name}. Validated 104-byte envelope.`);
      populateInspectorLocal(h, file.name);
    } catch (err) {
      selectedVaultBytes = null;
      unpackFilePreview.style.display = 'none';
      addLog('unpack-log', 'error', `Vault header error: ${err.message}`);
      showToast(err.message, 'error');
    }
  };
  reader.readAsArrayBuffer(file);
}

unpackFileInput.onchange = (e) => {
  if (e.target.files && e.target.files[0]) handleVaultFileSelect(e.target.files[0]);
  e.target.value = '';
};

unpackDropZone.ondragover = (e) => {
  e.preventDefault();
  unpackDropZone.classList.add('dragover');
};
unpackDropZone.ondragleave = () => unpackDropZone.classList.remove('dragover');
unpackDropZone.ondrop = (e) => {
  e.preventDefault();
  unpackDropZone.classList.remove('dragover');
  if (e.dataTransfer.files && e.dataTransfer.files[0]) handleVaultFileSelect(e.dataTransfer.files[0]);
};

unpackExecuteBtn.onclick = async () => {
  if (!selectedVaultBytes) {
    showToast('Please select a .vault archive first.', 'error');
    return;
  }

  const key = document.getElementById('unpack-key').value;
  if (!key) {
    showToast('Please enter the decryption passphrase.', 'error');
    return;
  }

  unpackExecuteBtn.disabled = true;

  if (isCppBackendOnline) {
    addLog('unpack-log', 'info', '[C++ BACKEND] Sending archive to C++ Native Engine (POST /api/unpack)...');
    try {
      const res = await fetch(`${apiBaseUrl}/api/unpack`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          passphrase: key,
          vaultData: uint8ToBase64(selectedVaultBytes)
        })
      });

      const data = await res.json();
      if (!res.ok || !data.success) {
        throw new Error(data.error || 'Decryption error on C++ backend.');
      }

      addLog('unpack-log', 'success', `[C++ BACKEND] Unlocked ${data.fileCount} file(s) in ${data.durationMs.toFixed(1)} ms. All hashes verified.`);
      showToast(`C++ Engine Decryption Successful: ${data.fileCount} file(s) restored.`, 'success');

      // Convert returned files
      currentDecryptedEntries = data.files.map(f => ({
        relativePath: f.path,
        fileSize: f.size,
        modifiedTime: f.mtime,
        sha256: f.sha256,
        data: base64ToUint8(f.data)
      }));

      renderCatalog(currentDecryptedEntries);
    } catch (err) {
      addLog('unpack-log', 'error', `[C++ BACKEND] Error: ${err.message}`);
      showToast(err.message, 'error');
    } finally {
      unpackExecuteBtn.disabled = false;
    }
  } else {
    // Fallback
    addLog('unpack-log', 'info', '[STANDALONE] Verifying and decrypting via Web Crypto fallback...');
    try {
      const startTime = performance.now();
      const h = parseVaultHeaderLocal(selectedVaultBytes);
      const { aesBytes, hmacBytes } = await deriveKeysFallback(key, h.salt, h.iterations);

      const ciphertext = selectedVaultBytes.subarray(HEADER_SIZE, HEADER_SIZE + h.ciphertextLength);
      const hmacData = new Uint8Array(PRE_HMAC_SIZE + ciphertext.length);
      hmacData.set(selectedVaultBytes.subarray(0, PRE_HMAC_SIZE), 0);
      hmacData.set(ciphertext, PRE_HMAC_SIZE);

      const hmacCryptoKey = await crypto.subtle.importKey('raw', hmacBytes, { name: 'HMAC', hash: 'SHA-256' }, false, ['sign']);
      const calculatedHmac = new Uint8Array(await crypto.subtle.sign('HMAC', hmacCryptoKey, hmacData));

      if (!constantTimeEqualFallback(calculatedHmac, h.expectedHmac)) {
        throw new Error("Authentication failed: Incorrect key or vault has been tampered with.");
      }

      const aesCryptoKey = await crypto.subtle.importKey('raw', aesBytes, { name: 'AES-CBC' }, false, ['decrypt']);
      const plaintext = new Uint8Array(await crypto.subtle.decrypt({ name: 'AES-CBC', iv: h.iv }, aesCryptoKey, ciphertext));
      const entries = await deserializePayloadFallback(plaintext);

      currentDecryptedEntries = entries;
      const duration = (performance.now() - startTime).toFixed(1);
      addLog('unpack-log', 'success', `Unlocked ${entries.length} file(s) in ${duration} ms.`);
      showToast(`Decryption successful: ${entries.length} file(s) restored.`, 'success');

      renderCatalog(currentDecryptedEntries);
    } catch (err) {
      addLog('unpack-log', 'error', `Decryption failed: ${err.message}`);
      showToast(err.message, 'error');
    } finally {
      unpackExecuteBtn.disabled = false;
    }
  }
};

function renderCatalog(entries) {
  unpackResultsCard.style.display = 'flex';
  unpackCountBadge.textContent = `${entries.length} file(s)`;
  unpackCatalogTbody.innerHTML = '';

  entries.forEach((entry, idx) => {
    const d = new Date(entry.modifiedTime * 1000);
    const dateStr = d.toISOString().replace('T', ' ').substring(0, 19);

    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td style="font-weight: 500;">${escapeHtml(entry.relativePath)}</td>
      <td class="text-right font-mono">${formatBytes(entry.fileSize)}</td>
      <td class="text-muted font-mono" style="font-size: 11px;">${dateStr}</td>
      <td class="font-mono text-muted" style="font-size: 11px;">${entry.sha256.substring(0, 16)}...</td>
      <td class="text-right">
        <button class="btn btn-sm download-single-btn" data-index="${idx}">
          <svg style="width:12px;height:12px;" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" y1="15" x2="12" y2="3"></line></svg>
          <span>Save</span>
        </button>
      </td>
    `;
    unpackCatalogTbody.appendChild(tr);
  });

  document.querySelectorAll('.download-single-btn').forEach(btn => {
    btn.onclick = () => {
      const i = parseInt(btn.dataset.index, 10);
      downloadSingleFile(currentDecryptedEntries[i]);
    };
  });
}

function downloadSingleFile(entry) {
  const blob = new Blob([entry.data]);
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = entry.relativePath.split('/').pop() || 'file';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

unpackDownloadAll.onclick = () => {
  if (!currentDecryptedEntries.length) return;
  const zipBlob = createZipBlob(currentDecryptedEntries);
  const url = URL.createObjectURL(zipBlob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'extracted_vault.zip';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
  showToast('All files downloaded as ZIP archive.', 'success');
};

// -----------------------------------------------------------------------------
// Tab 3: Verify Integrity
// -----------------------------------------------------------------------------
let verifyVaultBytes = null;
const verifyDropZone = document.getElementById('verify-drop-zone');
const verifyFileInput = document.getElementById('verify-file-input');
const verifyExecuteBtn = document.getElementById('verify-execute-btn');
const verifyStatusBanner = document.getElementById('verify-status-banner');

function handleVerifyFile(file) {
  const reader = new FileReader();
  reader.onload = () => {
    verifyVaultBytes = new Uint8Array(reader.result);
    showToast(`Loaded "${file.name}" for verification.`, 'info');
  };
  reader.readAsArrayBuffer(file);
}

verifyFileInput.onchange = (e) => {
  if (e.target.files && e.target.files[0]) handleVerifyFile(e.target.files[0]);
  e.target.value = '';
};
verifyDropZone.ondragover = (e) => { e.preventDefault(); verifyDropZone.classList.add('dragover'); };
verifyDropZone.ondragleave = () => verifyDropZone.classList.remove('dragover');
verifyDropZone.ondrop = (e) => {
  e.preventDefault();
  verifyDropZone.classList.remove('dragover');
  if (e.dataTransfer.files && e.dataTransfer.files[0]) handleVerifyFile(e.dataTransfer.files[0]);
};

verifyExecuteBtn.onclick = async () => {
  if (!verifyVaultBytes) {
    showToast('Please select a .vault file to verify.', 'error');
    return;
  }
  const key = document.getElementById('verify-key').value;
  if (!key) {
    showToast('Enter passphrase to verify.', 'error');
    return;
  }

  if (isCppBackendOnline) {
    try {
      const res = await fetch(`${apiBaseUrl}/api/verify`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ passphrase: key, vaultData: uint8ToBase64(verifyVaultBytes) })
      });
      const data = await res.json();
      verifyStatusBanner.style.display = 'flex';
      if (data.success && data.verified) {
        verifyStatusBanner.className = 'status-banner success';
        verifyStatusBanner.innerHTML = `
          <svg class="banner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>
          <div>
            <div style="font-weight:600;">[C++ Backend] Verification Passed</div>
            <div style="font-size:12px;">The passphrase is valid and the archive HMAC signature is intact.</div>
          </div>
        `;
      } else {
        verifyStatusBanner.className = 'status-banner error';
        verifyStatusBanner.innerHTML = `
          <svg class="banner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>
          <div>
            <div style="font-weight:600;">[C++ Backend] Verification Failed</div>
            <div style="font-size:12px;">Incorrect passphrase or archive has been modified.</div>
          </div>
        `;
      }
    } catch (err) {
      verifyStatusBanner.style.display = 'flex';
      verifyStatusBanner.className = 'status-banner error';
      verifyStatusBanner.textContent = `Error: ${err.message}`;
    }
  } else {
    // Local fallback
    try {
      const h = parseVaultHeaderLocal(verifyVaultBytes);
      const { hmacBytes } = await deriveKeysFallback(key, h.salt, h.iterations);
      const ciphertext = verifyVaultBytes.subarray(HEADER_SIZE, HEADER_SIZE + h.ciphertextLength);
      const hmacData = new Uint8Array(PRE_HMAC_SIZE + ciphertext.length);
      hmacData.set(verifyVaultBytes.subarray(0, PRE_HMAC_SIZE), 0);
      hmacData.set(ciphertext, PRE_HMAC_SIZE);

      const hmacKey = await crypto.subtle.importKey('raw', hmacBytes, { name: 'HMAC', hash: 'SHA-256' }, false, ['sign']);
      const computed = new Uint8Array(await crypto.subtle.sign('HMAC', hmacKey, hmacData));

      verifyStatusBanner.style.display = 'flex';
      if (constantTimeEqualFallback(computed, h.expectedHmac)) {
        verifyStatusBanner.className = 'status-banner success';
        verifyStatusBanner.innerHTML = `
          <svg class="banner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>
          <div>
            <div style="font-weight:600;">Verification Passed</div>
            <div style="font-size:12px;">Passphrase is valid and the archive envelope + ciphertext are 100% authentic.</div>
          </div>
        `;
      } else {
        verifyStatusBanner.className = 'status-banner error';
        verifyStatusBanner.innerHTML = `
          <svg class="banner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>
          <div>
            <div style="font-weight:600;">Verification Failed</div>
            <div style="font-size:12px;">Incorrect passphrase or archive has been modified.</div>
          </div>
        `;
      }
    } catch (err) {
      verifyStatusBanner.style.display = 'flex';
      verifyStatusBanner.className = 'status-banner error';
      verifyStatusBanner.textContent = `Error: ${err.message}`;
    }
  }
};

// -----------------------------------------------------------------------------
// Tab 4: Header Inspector
// -----------------------------------------------------------------------------
const inspectDropZone = document.getElementById('inspect-drop-zone');
const inspectFileInput = document.getElementById('inspect-file-input');
const inspectorGridWrapper = document.getElementById('inspector-grid-wrapper');

function populateInspectorLocal(h, filename) {
  inspectorGridWrapper.style.display = 'grid';
  document.getElementById('inspect-magic').textContent = 'SVAULT (0x535641554C54)';
  document.getElementById('inspect-version').textContent = h.version;
  document.getElementById('inspect-cipher').textContent = `AES-256-CBC (ID ${h.cipherId})`;
  document.getElementById('inspect-kdf').textContent = `PBKDF2-HMAC-SHA256 (ID ${h.kdfId})`;
  document.getElementById('inspect-iterations').textContent = `${h.iterations.toLocaleString()} rounds`;
  document.getElementById('inspect-size').textContent = `${formatBytes(h.ciphertextLength)} (${h.ciphertextLength} bytes)`;
  document.getElementById('inspect-salt').textContent = toHex(h.salt);
  document.getElementById('inspect-iv').textContent = toHex(h.iv);
  document.getElementById('inspect-hmac').textContent = toHex(h.expectedHmac);
}

async function handleInspectFile(file) {
  const reader = new FileReader();
  reader.onload = async () => {
    try {
      const bytes = new Uint8Array(reader.result);
      if (isCppBackendOnline) {
        const res = await fetch(`${apiBaseUrl}/api/info`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ vaultData: uint8ToBase64(bytes) })
        });
        const d = await res.json();
        if (d.success) {
          inspectorGridWrapper.style.display = 'grid';
          document.getElementById('inspect-magic').textContent = `${d.magic} (0x535641554C54)`;
          document.getElementById('inspect-version').textContent = d.version;
          document.getElementById('inspect-cipher').textContent = `${d.cipherName} (ID ${d.cipherId})`;
          document.getElementById('inspect-kdf').textContent = `${d.kdfName} (ID ${d.kdfId})`;
          document.getElementById('inspect-iterations').textContent = `${d.kdfIterations.toLocaleString()} rounds`;
          document.getElementById('inspect-size').textContent = `${formatBytes(d.ciphertextLength)} (${d.ciphertextLength} bytes)`;
          document.getElementById('inspect-salt').textContent = d.salt;
          document.getElementById('inspect-iv').textContent = d.iv;
          document.getElementById('inspect-hmac').textContent = d.hmac;
          showToast('Header metadata parsed by C++ Backend.', 'info');
          return;
        }
      }
      const h = parseVaultHeaderLocal(bytes);
      populateInspectorLocal(h, file.name);
      showToast('Header metadata parsed locally.', 'info');
    } catch (err) {
      showToast(err.message, 'error');
    }
  };
  reader.readAsArrayBuffer(file);
}

inspectFileInput.onchange = (e) => {
  if (e.target.files && e.target.files[0]) handleInspectFile(e.target.files[0]);
  e.target.value = '';
};
inspectDropZone.ondragover = (e) => { e.preventDefault(); inspectDropZone.classList.add('dragover'); };
inspectDropZone.ondragleave = () => inspectDropZone.classList.remove('dragover');
inspectDropZone.ondrop = (e) => {
  e.preventDefault();
  inspectDropZone.classList.remove('dragover');
  if (e.dataTransfer.files && e.dataTransfer.files[0]) handleInspectFile(e.dataTransfer.files[0]);
};

// -----------------------------------------------------------------------------
// Fallback Crypto Routines (For when running standalone offline without server)
// -----------------------------------------------------------------------------
function constantTimeEqualFallback(a, b) {
  if (a.length !== b.length) return false;
  let diff = 0;
  for (let i = 0; i < a.length; i++) diff |= (a[i] ^ b[i]);
  return diff === 0;
}

async function deriveKeysFallback(passphrase, salt, iterations) {
  const enc = new TextEncoder();
  const passKey = await crypto.subtle.importKey('raw', enc.encode(passphrase), 'PBKDF2', false, ['deriveBits']);
  const derivedBits = await crypto.subtle.deriveBits({ name: 'PBKDF2', salt, iterations, hash: 'SHA-256' }, passKey, 512);
  const masterBytes = new Uint8Array(derivedBits);
  return { aesBytes: masterBytes.slice(0, 32), hmacBytes: masterBytes.slice(32, 64) };
}

async function serializePayloadFallback(files) {
  const enc = new TextEncoder();
  let totalLength = 4;
  const prepared = [];
  for (const f of files) {
    const pathBytes = enc.encode(f.relativePath);
    const sha256 = new Uint8Array(await crypto.subtle.digest('SHA-256', f.data));
    totalLength += 4 + pathBytes.length + 8 + 8 + 32 + f.data.length;
    prepared.push({ pathBytes, fileSize: f.data.length, modifiedTime: f.modifiedTime || Math.floor(Date.now() / 1000), sha256, data: f.data });
  }

  const buffer = new Uint8Array(totalLength);
  const view = new DataView(buffer.buffer);
  let offset = 0;
  view.setUint32(offset, prepared.length, false);
  offset += 4;
  for (const p of prepared) {
    view.setUint32(offset, p.pathBytes.length, false); offset += 4;
    buffer.set(p.pathBytes, offset); offset += p.pathBytes.length;
    view.setBigUint64(offset, BigInt(p.fileSize), false); offset += 8;
    view.setBigUint64(offset, BigInt(p.modifiedTime), false); offset += 8;
    buffer.set(p.sha256, offset); offset += 32;
    buffer.set(p.data, offset); offset += p.data.length;
  }
  return buffer;
}

async function deserializePayloadFallback(plaintextBytes) {
  const view = new DataView(plaintextBytes.buffer, plaintextBytes.byteOffset, plaintextBytes.byteLength);
  let offset = 0;
  const dec = new TextDecoder('utf-8');
  if (view.byteLength < 4) throw new Error("Payload too short.");
  const count = view.getUint32(offset, false);
  offset += 4;
  const entries = [];
  for (let i = 0; i < count; i++) {
    const pathLen = view.getUint32(offset, false); offset += 4;
    const relPath = dec.decode(plaintextBytes.subarray(offset, offset + pathLen)); offset += pathLen;
    const fileSize = Number(view.getBigUint64(offset, false)); offset += 8;
    const mtime = Number(view.getBigUint64(offset, false)); offset += 8;
    const expectedHash = plaintextBytes.subarray(offset, offset + 32); offset += 32;
    const data = plaintextBytes.slice(offset, offset + fileSize); offset += fileSize;
    const actualHash = new Uint8Array(await crypto.subtle.digest('SHA-256', data));
    if (!constantTimeEqualFallback(actualHash, expectedHash)) {
      throw new Error(`Integrity failed for "${relPath}".`);
    }
    entries.push({ relativePath: relPath, fileSize, modifiedTime: mtime, sha256: toHex(expectedHash), data });
  }
  return entries;
}

// -----------------------------------------------------------------------------
// Global Navigation & Password Toggles
// -----------------------------------------------------------------------------
document.querySelectorAll('.tab-btn').forEach(btn => {
  btn.onclick = () => {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
    btn.classList.add('active');
    const target = document.getElementById(btn.dataset.tab);
    if (target) target.classList.add('active');
  };
});

document.querySelectorAll('.input-toggle-btn').forEach(btn => {
  btn.onclick = () => {
    const targetId = btn.dataset.target;
    const input = document.getElementById(targetId);
    if (!input) return;
    if (input.type === 'password') {
      input.type = 'text';
      btn.innerHTML = `<svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line></svg>`;
    } else {
      input.type = 'password';
      btn.innerHTML = `<svg class="input-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>`;
    }
  };
});

// Run backend health check on load
checkBackendStatus();
