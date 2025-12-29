# 🔐 Cipher

<div align="center">

![Cipher Logo](https://img.shields.io/badge/Cipher-Password_Manager-blue?style=for-the-badge&logo=lock)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)](https://github.com/matheusc457/cipher)

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Documentation](#-documentation) • [Contributing](#-contributing)

</div>

---

## 📋 Overview

**Cipher** is a terminal-based password manager that keeps your credentials safe using strong encryption. Store all your passwords securely behind a single master password.

### Why Cipher?

- 🔒 **Secure**: AES encryption to protect your data
- 🚀 **Fast**: Lightweight C implementation
- 🎯 **Simple**: Clean CLI interface
- 🔓 **Open Source**: Transparent and auditable code
- 💾 **Portable**: Single encrypted file for all your passwords

---

## ✨ Features

- ✅ **Master Password Protection**: One password to rule them all
- ✅ **Strong Encryption**: AES-256 encryption for stored passwords
- ✅ **Password Generator**: Create strong random passwords
- ✅ **CRUD Operations**: Add, search, update, and delete passwords
- ✅ **Password Strength Analyzer**: Check if your passwords are strong
- ✅ **Secure Storage**: Encrypted file-based storage
- ✅ **Cross-Platform**: Works on Linux, macOS, and Windows

### 🚧 Upcoming Features

- [ ] Password expiration reminders
- [ ] Two-factor authentication (2FA) storage
- [ ] Import/Export functionality
- [ ] Clipboard integration with auto-clear
- [ ] Password history tracking
- [ ] Multi-user support

---

## 🛠️ Installation

### Prerequisites

- GCC compiler (or any C compiler)
- Make
- OpenSSL development libraries (for encryption)

### Linux/macOS

```bash
# Install OpenSSL (if not already installed)
# Ubuntu/Debian:
sudo apt-get install libssl-dev

# macOS:
brew install openssl

# Clone the repository
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
make
./bin/cipher.exe
```

---

## 🚀 Usage

### First Time Setup

```bash
$ ./bin/cipher

Welcome to Cipher! 🔐
No password file found. Creating new vault...
Enter your master password: ********
Confirm master password: ********
✅ Vault created successfully!
```

### Main Menu

```plaintext
╔══════════════════════════════════════╗
║        CIPHER PASSWORD MANAGER       ║
╚══════════════════════════════════════╝

[1] Add new password
[2] Search password
[3] List all services
[4] Update password
[5] Delete password
[6] Generate strong password
[7] Change master password
[8] Exit

Choose an option: _
```

### Example Usage

#### Adding a password

```plaintext
Choose an option: 1

Service name: Gmail
Username: john@example.com
Password: ******** (or press G to generate)

✅ Password saved successfully!
```

#### Searching for a password

```plaintext
Choose an option: 2

Service name: Gmail

📧 Gmail
   Username: john@example.com
   Password: MySecurePass123
   
🔒 Password copied to clipboard (will clear in 30s)
```

#### Generating a strong password

```plaintext
Choose an option: 6

Password length (8-32): 16
Include uppercase letters? (y/n): y
Include numbers? (y/n): y
Include symbols? (y/n): y

Generated password: K#9mP@xL2$qR7nWz
Strength: 🟢 STRONG

Copy to clipboard? (y/n): y
✅ Copied!
```

---

## 📁 Project Structure

```plaintext
cipher/
├── src/
│   ├── main.c           # Entry point and main menu
│   ├── crypto.c/h       # Encryption/decryption functions
│   ├── password.c/h     # Password management logic
│   ├── generator.c/h    # Password generator
│   ├── file_io.c/h      # File operations
│   └── utils.c/h        # Utility functions
├── data/
│   └── passwords.dat    # Encrypted password storage (created at runtime)
├── docs/
│   └── architecture.md  # Technical documentation
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## 🔒 Security

### Encryption

- **Algorithm**: AES-256-CBC
- **Key Derivation**: PBKDF2 with SHA-256
- **Salt**: Random 16-byte salt per file
- **Iterations**: 100,000 iterations for key derivation

### Best Practices

- ⚠️ Never share your master password
- ⚠️ Use a strong master password (12+ characters)
- ⚠️ Keep backups of your `passwords.dat` file
- ⚠️ This is an educational project - use at your own risk

### Limitations

This is a learning project and should not be used for critical production environments. For production use, consider established solutions like:

- [Bitwarden](https://bitwarden.com/)
- [KeePassXC](https://keepassxc.org/)
- [1Password](https://1password.com/)

---

## 📚 Documentation

Detailed documentation is available in the `docs/` directory:

- Architecture Overview
- API Reference (coming soon)
- Security Design (coming soon)

---

## 🧪 Building from Source

```bash
# Debug build
make debug

# Release build (optimized)
make release

# Run tests
make test

# Clean build files
make clean
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

- Follow C11 standard
- Use 4 spaces for indentation
- Comment complex logic
- Keep functions under 50 lines when possible

---

## 📝 License

This project is licensed under the GNU General Public License v3.0 (GPLv3). See the [LICENSE](LICENSE) file for the full license text or visit the [GNU GPLv3 page](https://www.gnu.org/licenses/gpl-3.0.en.html) for details.

---

## 👨‍💻 Author

Created with ❤️ by Matheus

---

## 🙏 Acknowledgments

- Inspired by password managers like KeePass and Bitwarden
- Built as a learning project to understand C programming and cryptography
- Thanks to the open-source community

---

⭐ **If you find this project useful, please consider giving it a star!**

---

⬆ [Back to top](#readme)

Made with 🔐 and C.
