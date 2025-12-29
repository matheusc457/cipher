# Cipher Password Manager - Makefile
# Compiler and flags
CC = gcc
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

# Object files
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

# Create necessary directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DATA_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile source files to object files
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/password.h $(SRC_DIR)/generator.h $(SRC_DIR)/passphrase.h $(SRC_DIR)/crypto.h
	@echo "Compiling main.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/crypto.o: $(SRC_DIR)/crypto.c $(SRC_DIR)/crypto.h
	@echo "Compiling crypto.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/crypto.c -o $(OBJ_DIR)/crypto.o

$(OBJ_DIR)/password.o: $(SRC_DIR)/password.c $(SRC_DIR)/password.h $(SRC_DIR)/crypto.h $(SRC_DIR)/file_io.h
	@echo "Compiling password.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/password.c -o $(OBJ_DIR)/password.o

$(OBJ_DIR)/generator.o: $(SRC_DIR)/generator.c $(SRC_DIR)/generator.h $(SRC_DIR)/utils.h
	@echo "Compiling generator.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/generator.c -o $(OBJ_DIR)/generator.o

$(OBJ_DIR)/passphrase.o: $(SRC_DIR)/passphrase.c $(SRC_DIR)/passphrase.h $(SRC_DIR)/utils.h
	@echo "Compiling passphrase.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/passphrase.c -o $(OBJ_DIR)/passphrase.o

$(OBJ_DIR)/file_io.o: $(SRC_DIR)/file_io.c $(SRC_DIR)/file_io.h
	@echo "Compiling file_io.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/file_io.c -o $(OBJ_DIR)/file_io.o

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h
	@echo "Compiling utils.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utils.c -o $(OBJ_DIR)/utils.o

# Debug build with symbols
debug: CFLAGS += -g -DDEBUG
debug: clean all
	@echo "Debug build complete!"

# Release build with optimizations
release: CFLAGS += -O2 -DNDEBUG
release: clean all
	@echo "Release build complete!"

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "Clean complete!"

# Clean everything including data
distclean: clean
	@echo "Cleaning all generated files..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo "Distclean complete!"

# Install target (optional)
install: $(TARGET)
	@echo "Installing cipher to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/cipher
	@echo "Installation complete!"

# Uninstall target (optional)
uninstall:
	@echo "Uninstalling cipher..."
	sudo rm -f /usr/local/bin/cipher
	@echo "Uninstall complete!"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Check if wordlist exists
check-wordlist:
	@if [ ! -f "$(DATA_DIR)/eff_large_wordlist.txt" ]; then \
		echo "⚠️  Wordlist not found!"; \
		echo "Download it with:"; \
		echo "  cd $(DATA_DIR) && wget https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt"; \
		exit 1; \
	else \
		echo "✅ Wordlist found!"; \
	fi

# Help target
help:
	@echo "Cipher Password Manager - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build the project (default)"
	@echo "  debug        - Build with debug symbols"
	@echo "  release      - Build with optimizations"
	@echo "  clean        - Remove object files and executable"
	@echo "  distclean    - Remove all generated files"
	@echo "  run          - Build and run the program"
	@echo "  install      - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall    - Remove from /usr/local/bin (requires sudo)"
	@echo "  check-wordlist - Verify wordlist file exists"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Example usage:"
	@echo "  make              # Build the project"
	@echo "  make debug        # Build with debug info"
	@echo "  make run          # Build and run"
	@echo "  make clean        # Clean build files"

# Phony targets
.PHONY: all clean distclean debug release run install uninstall check-wordlist help directories
