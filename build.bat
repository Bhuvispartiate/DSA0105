@echo off
setlocal

echo ========================================================
echo Building SecureVault Project
echo ========================================================

where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    if exist "%USERPROFILE%\w64devkit\bin\g++.exe" (
        set "PATH=%USERPROFILE%\w64devkit\bin;%PATH%"
    ) else (
        echo Error: g++ compiler not found in PATH or %USERPROFILE%\w64devkit\bin
        exit /b 1
    )
)

echo Compiling SecureVault CLI and C++ Server (vault.exe)...
g++ -std=c++17 -Wall -Wextra -O3 -Iinclude -o vault.exe src/main.cpp src/HttpServer.cpp src/Crypto.cpp src/VaultFormat.cpp src/VaultWriter.cpp src/VaultReader.cpp src/FileUtils.cpp -ladvapi32 -lws2_32 -lshell32
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed for vault.exe!
    exit /b %ERRORLEVEL%
)

echo Compiling Test Suite (test_vault.exe)...
g++ -std=c++17 -Wall -Wextra -O3 -Iinclude -o test_vault.exe tests/test_vault.cpp src/Crypto.cpp src/VaultFormat.cpp src/VaultWriter.cpp src/VaultReader.cpp src/FileUtils.cpp -ladvapi32
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed for test_vault.exe!
    exit /b %ERRORLEVEL%
)

echo.
echo Build successful!
echo Executables created:
echo   - vault.exe      (Main CLI Application)
echo   - test_vault.exe (Automated Test Suite)
echo.

