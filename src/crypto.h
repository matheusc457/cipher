#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>

#define KEY_SIZE 32         // 256 bits
#define IV_SIZE 16          // 128 bits
#define SALT_SIZE 16        // 128 bits
#define HASH_SIZE 32        // SHA-256 output

// Initialize crypto library
int crypto_init(void);

// Cleanup crypto library
void crypto_cleanup(void);

// Derive key from master password using PBKDF2
int derive_key(const char *password, const unsigned char *salt, 
               unsigned char *key, size_t key_len);

// Hash master password for verification
int hash_password(const char *password, const unsigned char *salt,
                  unsigned char *hash, size_t hash_len);

// Encrypt data using AES-256-CBC
int encrypt_data(const unsigned char *plaintext, size_t plaintext_len,
                 const unsigned char *key, const unsigned char *iv,
                 unsigned char *ciphertext, size_t *ciphertext_len);

// Decrypt data using AES-256-CBC
int decrypt_data(const unsigned char *ciphertext, size_t ciphertext_len,
                 const unsigned char *key, const unsigned char *iv,
                 unsigned char *plaintext, size_t *plaintext_len);

// Generate random bytes
int generate_random_bytes(unsigned char *buffer, size_t length);

// Verify master password
int verify_master_password(const char *password, const unsigned char *salt,
                          const unsigned char *stored_hash);

#endif // CRYPTO_H
