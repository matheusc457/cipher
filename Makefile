# Cipher Password Manager - Makefile

# Compiler selection (default: gcc, use 'make CC=clang' for clang)
CC ?= gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src
LDFLAGS = -lssl -lcrypto -lm

# Directories
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
DATA_DIR = data

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/crypto.c \
          $(SRC_DIR)/password.c \
          $(SRC_DIR)/generator.c \
          $(SRC_DIR)/passphrase.c \
          $(SRC_DIR)/file_io.c \
          $(SRC_DIR)/utils.c

OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/crypto.o \
          $(OBJ_DIR)/password.o \
          $(OBJ_DIR)/generator.o \
          $(OBJ_DIR)/passphrase.o \
          $(OBJ_DIR)/file_io.o \
          $(OBJ_DIR)/utils.o

# Target executable
TARGET = $(BIN_DIR)/cipher

# Default target
all: directories $(TARGET)
	@echo "[SUCCESS] Build complete with $(CC)! Run with: ./$(TARGET)"

# Create necessary directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DATA_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET) with $(CC)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/password.h $(SRC_DIR)/generator.h $(SRC_DIR)/passphrase.h $(SRC_DIR)/crypto.h
	@echo "Compiling main.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/crypto.o: $(SRC_DIR)/crypto.c $(SRC_DIR)/crypto.h
	@echo "Compiling crypto.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/crypto.c -o $(OBJ_DIR)/crypto.o

$(OBJ_DIR)/password.o: $(SRC_DIR)/password.c $(SRC_DIR)/password.h $(SRC_DIR)/crypto.h $(SRC_DIR)/file_io.h
	@echo "Compiling password.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/password.c -o $(OBJ_DIR)/password.o

$(OBJ_DIR)/generator.o: $(SRC_DIR)/generator.c $(SRC_DIR)/generator.h $(SRC_DIR)/utils.h
	@echo "Compiling generator.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/generator.c -o $(OBJ_DIR)/generator.o

$(OBJ_DIR)/passphrase.o: $(SRC_DIR)/passphrase.c $(SRC_DIR)/passphrase.h $(SRC_DIR)/utils.h
	@echo "Compiling passphrase.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/passphrase.c -o $(OBJ_DIR)/passphrase.o

$(OBJ_DIR)/file_io.o: $(SRC_DIR)/file_io.c $(SRC_DIR)/file_io.h
	@echo "Compiling file_io.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/file_io.c -o $(OBJ_DIR)/file_io.o

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h
	@echo "Compiling utils.c with $(CC)..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utils.c -o $(OBJ_DIR)/utils.o

# Build with Clang
clang:
	@echo "Building with Clang..."
	@$(MAKE) CC=clang

# Build with GCC (explicit)
gcc:
	@echo "Building with GCC..."
	@$(MAKE) CC=gcc

# Debug build with symbols
debug: CFLAGS += -g -DDEBUG
debug: clean all
	@echo "[SUCCESS] Debug build complete with $(CC)!"

# Release build with optimizations
release: CFLAGS += -O2 -DNDEBUG
release: clean all
	@echo "[SUCCESS] Release build complete with $(CC)!"

# Clang with sanitizers (for development/debugging)
sanitize: CC = clang
sanitize: CFLAGS += -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
sanitize: LDFLAGS += -fsanitize=address -fsanitize=undefined
sanitize: clean all
	@echo "[SUCCESS] Sanitize build complete! (Address Sanitizer + UB Sanitizer)"

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "[SUCCESS] Clean complete!"

# Clean everything including data
distclean: clean
	@echo "Cleaning all generated files..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo "[SUCCESS] Distclean complete!"

# Install target (optional)
install: $(TARGET)
	@echo "Installing cipher to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/cipher
	@echo "[SUCCESS] Installation complete!"

# Uninstall target (optional)
uninstall:
	@echo "Uninstalling cipher..."
	sudo rm -f /usr/local/bin/cipher
	@echo "[SUCCESS] Uninstall complete!"

# Run the program
run: $(TARGET)
	./$(TARGET)

check-wordlist:
	@if [ ! -f "$(DATA_DIR)/eff_large_wordlist.txt" ]; then \
		echo "[WARNING] Wordlist not found!"; \
		echo "Download it with:"; \
		echo "  cd $(DATA_DIR) && wget https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt"; \
		exit 1; \
	else \
		echo "[SUCCESS] Wordlist found!"; \
	fi

# Show compiler info
compiler-info:
	@echo "=== Compiler Information ==="
	@echo "Current compiler: $(CC)"
	@which $(CC) 2>/dev/null || echo "$(CC) not found in PATH"
	@$(CC) --version 2>/dev/null || echo "Could not get $(CC) version"
	@echo ""
	@echo "=== Available Compilers ==="
	@which gcc 2>/dev/null && gcc --version | head -n1 || echo "GCC: not found"
	@which clang 2>/dev/null && clang --version | head -n1 || echo "Clang: not found"

# Help target
help:
	@echo "Cipher Password Manager - Makefile"
	@echo ""
	@echo "=== Build Targets ==="
	@echo "  make              - Build with default compiler (gcc)"
	@echo "  make gcc          - Build explicitly with GCC"
	@echo "  make clang        - Build explicitly with Clang"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make release      - Build optimized release version"
	@echo "  make sanitize     - Build with Clang sanitizers (dev/debug)"
	@echo ""
	@echo "=== Utility Targets ==="
	@echo "  make clean        - Remove object files and executable"
	@echo "  make distclean    - Remove all generated files"
	@echo "  make run          - Build and run the program"
	@echo "  make install      - Install to /usr/local/bin (requires sudo)"
	@echo "  make uninstall    - Remove from /usr/local/bin (requires sudo)"
	@echo "  make check-wordlist - Verify wordlist file exists"
	@echo "  make compiler-info  - Show available compilers"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "=== Advanced Usage ==="
	@echo "  make CC=clang     - Build with specific compiler"
	@echo "  make CC=clang release - Clang optimized build"
	@echo "  make CFLAGS=\"-O3 -march=native\" - Custom flags"
	@echo ""
	@echo "=== Examples ==="
	@echo "  make              # Build with GCC"
	@echo "  make clang        # Build with Clang"
	@echo "  make debug        # Debug build with current compiler"
	@echo "  make CC=clang debug # Debug build with Clang"
	@echo "  make sanitize     # Development build with sanitizers"
	@echo "  make clean run    # Clean and run"

# Phony targets
.PHONY: all clean distclean debug release run install uninstall check-wordlist help directories gcc clang sanitize compiler-info
