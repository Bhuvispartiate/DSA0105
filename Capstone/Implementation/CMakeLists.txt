cmake_minimum_required(VERSION 3.16)
project(SecureVault VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /EHsc /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

include_directories(include)

set(CORE_SOURCES
    src/Crypto.cpp
    src/VaultFormat.cpp
    src/VaultWriter.cpp
    src/VaultReader.cpp
    src/FileUtils.cpp
    src/HttpServer.cpp
)

# CLI Application & C++ Backend Server
add_executable(vault src/main.cpp ${CORE_SOURCES})

# Test Runner
add_executable(test_vault tests/test_vault.cpp src/Crypto.cpp src/VaultFormat.cpp src/VaultWriter.cpp src/VaultReader.cpp src/FileUtils.cpp)

# Windows libraries
if(WIN32)
    target_link_libraries(vault PRIVATE advapi32 ws2_32 shell32)
    target_link_libraries(test_vault PRIVATE advapi32)
endif()

enable_testing()
add_test(NAME VaultAutomatedTests COMMAND test_vault)

