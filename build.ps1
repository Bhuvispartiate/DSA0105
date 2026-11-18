# SecureVault PowerShell Build Script
$ErrorActionPreference = "Stop"

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "Building SecureVault Project (PowerShell)" -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    if (Test-Path "$env:USERPROFILE\w64devkit\bin\g++.exe") {
        $env:PATH = "$env:USERPROFILE\w64devkit\bin;" + $env:PATH
    } else {
        Write-Error "g++ compiler not found in PATH or $env:USERPROFILE\w64devkit\bin"
        exit 1
    }
}

$coreSrcs = @(
    "src/Crypto.cpp",
    "src/VaultFormat.cpp",
    "src/VaultWriter.cpp",
    "src/VaultReader.cpp",
    "src/FileUtils.cpp"
)

Write-Host "Compiling vault.exe (CLI + C++ Backend Server)..." -ForegroundColor Green
& g++ -std=c++17 -Wall -Wextra -O3 -Iinclude -o vault.exe src/main.cpp src/HttpServer.cpp @coreSrcs -ladvapi32 -lws2_32 -lshell32
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for vault.exe"
    exit $LASTEXITCODE
}

Write-Host "Compiling test_vault.exe..." -ForegroundColor Green
& g++ -std=c++17 -Wall -Wextra -O3 -Iinclude -o test_vault.exe tests/test_vault.cpp @coreSrcs -ladvapi32
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed for test_vault.exe"
    exit $LASTEXITCODE
}

Write-Host "`nBuild successful!" -ForegroundColor Cyan
Write-Host "  vault.exe      - CLI Application"
Write-Host "  test_vault.exe - Test Suite"

