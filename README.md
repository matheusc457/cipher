# 🔐 Cipher

[![Cipher Logo](https://img.shields.io/badge/Cipher-Password_Manager-blue?style=for-the-badge&logo=lock)](https://github.com/matheusc457/cipher)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)](https://github.com/matheusc457/cipher)

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Passphrase Generator](#-passphrase-generator-new) • [Documentation](#-documentation) • [Contributing](#-contributing)

---

## 📋 Overview

**Cipher** is a terminal-based password manager that keeps your credentials safe using strong encryption. Store all your passwords securely behind a single master password.

### Why Cipher?

* 🔒 **Secure**: AES-256 encryption to protect your data
* 🚀 **Fast**: Lightweight C implementation
* 🎯 **Simple**: Clean CLI interface
* 🔓 **Open Source**: Transparent and auditable code
* 💾 **Portable**: Single encrypted file for all your passwords
* 🎲 **Smart Generation**: Random passwords AND memorable passphrases

---

## ✨ Features

* ✅ **Master Password Protection**: One password to rule them all
* ✅ **Strong Encryption**: AES-256-CBC encryption for stored passwords
* ✅ **Dual Password Generation**:
  * 🔤 **Random Character Generator**: Strong random passwords (e.g., `K#9mP@xL2$qR7nWz`)
  * 🎲 **Passphrase Generator**: Memorable word-based passwords (e.g., `correct-horse-battery-staple`)
* ✅ **CRUD Operations**: Add, search, update, and delete passwords
* ✅ **Password Strength Analyzer**: Check if your passwords are strong
* ✅ **Secure Storage**: Encrypted file-based storage
* ✅ **Cross-Platform**: Works on Linux, macOS, and Windows
* ✅ **Entropy Calculation**: Know exactly how secure your passwords are

### 🚧 Upcoming Features

* Password expiration reminders
* Two-factor authentication (2FA) storage
* Import/Export functionality
* Clipboard integration with auto-clear
* Password history tracking
* Multi-user support
* Backup and restore system

---

## 🛠️ Installation

### Prerequisites

* GCC compiler (or any C compiler)
* Make
* OpenSSL development libraries (for encryption)

### Linux/macOS

```bash
# Install OpenSSL (if not already installed)
# Ubuntu/Debian:
sudo apt-get install libssl-dev

# macOS:
brew install openssl

# Clone the repository (wordlist included!)
git clone https://github.com/matheusc457/cipher.git
cd cipher

# Build the project
make

# Run Cipher
./bin/cipher
```

### Windows

Using MinGW or WSL:

```bash
# Install OpenSSL first, then:
git clone https://github.com/matheusc457/cipher.git
cd cipher

# Build and run
make
./bin/cipher.exe
```

---

## 🚀 Usage

### First Time Setup

```
$ ./bin/cipher

Welcome to Cipher!
No password file found. Creating new vault...
Enter your master password: ********
Confirm master password: ********
[SUCCESS] Vault created successfully!
```

### Main Menu

```
╔══════════════════════════════════════╗
║        CIPHER PASSWORD MANAGER       ║
╚══════════════════════════════════════╝

[1] Add new password
[2] Search password
[3] List all services
[4] Update password
[5] Delete password
[6] Generate random password
[7] Generate passphrase
[8] Change master password
[9] Exit

Choose an option: _
```

### Example Usage

#### Adding a password

```
Choose an option: 1

Service name: Gmail
Username: john@example.com

Password options:
  [1] Enter manually
  [2] Generate random password
  [3] Generate passphrase
Choose: 3

Passphrase presets:
  [1] Basic    - 3 words
  [2] Standard - 4 words (recommended)
  [3] Strong   - 5 words
Choose preset: 2

Generated passphrase: correct-horse-battery-staple
Entropy: 51.7 bits

[SUCCESS] Password added successfully!
```

#### Searching for a password

```
Choose an option: 2

Service name: Gmail

╔════════════════════════════════════════╗
║           Password Found               ║
╚════════════════════════════════════════╝

  Service:  Gmail
  Username: john@example.com
  Password: correct-horse-battery-staple
  
  Strength: STRONG
```

#### Generating a strong password

```
Choose an option: 6

Password length (8-32): 16
Include uppercase letters? (y/n): y
Include numbers? (y/n): y
Include symbols? (y/n): y

Generated password: K#9mP@xL2$qR7nWz
Strength: STRONG
```

---

## 🎲 Passphrase Generator (NEW!)

Generate memorable, secure passphrases using the EFF wordlist.

### Usage Example

```
Choose an option: 7

╔══════════════════════════════════════╗
║    PASSPHRASE GENERATOR              ║
╚══════════════════════════════════════╝

Choose strength level:

[1] Basic    - 3 words (39 bits)
    Example: apple-tree-cloud
    Crack time: ~1 year
    
[2] Standard - 4 words (52 bits)
    Example: apple-tree-cloud-river
    Crack time: ~71 years
    
[3] Strong   - 5 words (65 bits)
    Example: apple-tree-cloud-river-moon
    Crack time: ~584k years
    
[4] Maximum  - 6 words (77 bits)
    Example: apple-tree-cloud-river-moon-star
    Crack time: ~4.7M years
    
[5] Custom   - Configure manually

[0] Back to main menu

Choice: 2

Generating passphrase...

  correct-horse-battery-staple

Strength: ######---- STRONG
Entropy: 51.7 bits
Crack time: ~71 years

What would you like to do?
[1] Generate another
[2] Copy to clipboard (manual)
[3] Back
```

### Why Passphrases?

Passphrases like `correct-horse-battery-staple` are:
- ✅ **Easier to remember** than random characters
- ✅ **Just as secure** when using enough words
- ✅ **Faster to type** without special characters
- ✅ **Less prone to typos**

**Security Comparison:**
- `K#9mP@xL2$qR7nWz` (16 chars) ≈ 95 bits entropy
- `correct-horse-battery-staple` (4 words) ≈ 52 bits entropy
- `correct-horse-battery-staple-mountain` (5 words) ≈ 65 bits entropy

---

## 📁 Project Structure

```
cipher/
├── src/
│   ├── main.c           # Entry point and main menu
│   ├── crypto.c/h       # Encryption/decryption functions
│   ├── password.c/h     # Password management logic
│   ├── generator.c/h    # Random password generator
│   ├── passphrase.c/h   # Passphrase generator (NEW!)
│   ├── file_io.c/h      # File operations
│   └── utils.c/h        # Utility functions
├── data/
│   ├── passwords.dat              # Encrypted password storage
│   └── eff_large_wordlist.txt     # EFF wordlist (7,776 words)
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## 🔒 Security

### Encryption

* **Algorithm**: AES-256-CBC
* **Key Derivation**: PBKDF2 with SHA-256
* **Salt**: Random 16-byte salt per file
* **Iterations**: 100,000 iterations for key derivation
* **Random Generation**: OpenSSL RAND_bytes for cryptographically secure randomness

### Best Practices

* ⚠️ Never share your master password
* ⚠️ Use a strong master password (12+ characters or 4+ word passphrase)
* ⚠️ Keep backups of your `passwords.dat` file
* ⚠️ This is an educational project - use at your own risk

### Limitations

This is a learning project and should not be used for critical production environments. For production use, consider established solutions like:

* [Bitwarden](https://bitwarden.com/)
* [KeePassXC](https://keepassxc.org/)
* [1Password](https://1password.com/)

---

## 📚 Third-Party Resources

This project uses the **EFF Large Wordlist** for passphrase generation.

* **Source:** [Electronic Frontier Foundation](https://www.eff.org/dice)
* **Download:** https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt
* **License:** [Creative Commons Attribution 3.0 United States](https://creativecommons.org/licenses/by/3.0/us/)
* **File:** `data/eff_large_wordlist.txt` (7,776 words)

The wordlist was created by Joseph Bonneau and EFF, designed specifically for creating strong, memorable passphrases.

**Attribution:** This project includes the EFF Large Wordlist, © 2016 Electronic Frontier Foundation, licensed under CC BY 3.0 US.

---

## 🧪 Building from Source

### Compiler Options

Cipher supports both **GCC** and **Clang** compilers:

```bash
# Build with GCC (default)
make

# Build with Clang
make clang

# Build with specific compiler
make CC=clang
make CC=gcc-13
```

### Build Types

```bash
# Debug build (with symbols)
make debug

# Release build (optimized)
make release

# Clang with sanitizers (development/debugging)
make sanitize
```

### Additional Targets

```bash
# Clean build files
make clean

# Clean everything
make distclean

# Check available compilers
make compiler-info

# Show all options
make help
```

### Advanced Examples

```bash
# Clang optimized build
make CC=clang release

# Custom optimization flags
make CFLAGS="-O3 -march=native"

# Debug build with Clang
make CC=clang debug
```

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Setup

```bash
# Fork the repository
# Create your feature branch
git checkout -b feature/AmazingFeature

# Commit your changes
git commit -m 'Add some AmazingFeature'

# Push to the branch
git push origin feature/AmazingFeature

# Open a Pull Request
```

### Code Style

* Follow C11 standard
* Use 4 spaces for indentation
* Comment complex logic
* Keep functions under 50 lines when possible

---

## 📝 License

This project is licensed under the GNU General Public License v3.0 (GPLv3). See the [LICENSE](LICENSE) file for the full license text or visit the [GNU GPLv3 page](https://www.gnu.org/licenses/gpl-3.0.en.html) for details.

---

## 👨‍💻 Author

Created with ❤️ by [Matheus](https://github.com/matheusc457)

---

## 🙏 Acknowledgments

* Inspired by password managers like KeePass and Bitwarden
* EFF Large Wordlist for passphrase generation
* Built as a learning project to understand C programming and cryptography
* Thanks to the open-source community

---

⭐ **If you find this project useful, please consider giving it a star!**

---

⬆ [Back to top](#-cipher)

Made with 🔐 and C
