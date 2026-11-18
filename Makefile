CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -Iinclude
LDFLAGS = -ladvapi32 -lws2_32 -lshell32

SRCS = src/Crypto.cpp src/VaultFormat.cpp src/VaultWriter.cpp src/VaultReader.cpp src/FileUtils.cpp src/HttpServer.cpp
OBJS = $(SRCS:.cpp=.o)

MAIN_SRC = src/main.cpp
MAIN_OBJ = src/main.o

TEST_SRC = tests/test_vault.cpp
TEST_OBJ = tests/test_vault.o

BIN_MAIN = vault.exe
BIN_TEST = test_vault.exe

.PHONY: all clean test

all: $(BIN_MAIN) $(BIN_TEST)

$(BIN_MAIN): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_TEST): $(OBJS) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(BIN_TEST)
	./$(BIN_TEST)

clean:
	rm -f $(OBJS) $(MAIN_OBJ) $(TEST_OBJ) $(BIN_MAIN) $(BIN_TEST)

