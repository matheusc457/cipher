# Cipher Password Manager Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lcrypto
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/cipher

# Colors for output
COLOR_RESET = \033[0m
COLOR_GREEN = \033[32m
COLOR_YELLOW = \033[33m
COLOR_CYAN = \033[36m

# Default target
all: $(TARGET)
	@echo "$(COLOR_GREEN)✅ Cipher compiled successfully!$(COLOR_RESET)"
	@echo "$(COLOR_CYAN)Run with: ./$(TARGET)$(COLOR_RESET)"

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	@echo "$(COLOR_YELLOW)Linking...$(COLOR_RESET)"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "$(COLOR_CYAN)Compiling $<...$(COLOR_RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)
	@echo "$(COLOR_GREEN)✅ Debug build complete!$(COLOR_RESET)"

# Release build (optimized)
release: CFLAGS += -O3
release: clean $(TARGET)
	@echo "$(COLOR_GREEN)✅ Release build complete!$(COLOR_RESET)"

# Clean build files
clean:
	@echo "$(COLOR_YELLOW)Cleaning build files...$(COLOR_RESET)"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "$(COLOR_GREEN)✅ Clean complete!$(COLOR_RESET)"

# Run the program
run: $(TARGET)
	@echo "$(COLOR_CYAN)Starting Cipher...$(COLOR_RESET)"
	@./$(TARGET)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	@echo "$(COLOR_YELLOW)Installing Cipher...$(COLOR_RESET)"
	@sudo cp $(TARGET) /usr/local/bin/cipher
	@sudo chmod +x /usr/local/bin/cipher
	@echo "$(COLOR_GREEN)✅ Cipher installed! Run with: cipher$(COLOR_RESET)"

# Uninstall
uninstall:
	@echo "$(COLOR_YELLOW)Uninstalling Cipher...$(COLOR_RESET)"
	@sudo rm -f /usr/local/bin/cipher
	@echo "$(COLOR_GREEN)✅ Cipher uninstalled!$(COLOR_RESET)"

# Help
help:
	@echo "$(COLOR_CYAN)Cipher Password Manager - Makefile Help$(COLOR_RESET)"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build the project (default)"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make release  - Build optimized release version"
	@echo "  make clean    - Remove build files"
	@echo "  make run      - Build and run the program"
	@echo "  make install  - Install to /usr/local/bin"
	@echo "  make uninstall- Remove from /usr/local/bin"
	@echo "  make help     - Show this help message"
	@echo ""

.PHONY: all debug release clean run install uninstall help
