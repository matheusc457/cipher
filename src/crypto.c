#include "crypto.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define PBKDF2_ITERATIONS 100000

int crypto_init(void) {
    OpenSSL_add_all_algorithms();
    return 1;
}

void crypto_cleanup(void) {
    EVP_cleanup();
}

int derive_key(const char *password, const unsigned char *salt,
               unsigned char *key, size_t key_len) {
    if (!password || !salt || !key) return 0;
    
    return PKCS5_PBKDF2_HMAC(password, strlen(password),
                             salt, SALT_SIZE,
                             PBKDF2_ITERATIONS,
                             EVP_sha256(),
                             key_len, key) == 1;
}

int hash_password(const char *password, const unsigned char *salt,
                  unsigned char *hash, size_t hash_len) {
    return derive_key(password, salt, hash, hash_len);
}

int encrypt_data(const unsigned char *plaintext, size_t plaintext_len,
                 const unsigned char *key, const unsigned char *iv,
                 unsigned char *ciphertext, size_t *ciphertext_len) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int total_len = 0;
    
    if (!(ctx = EVP_CIPHER_CTX_new())) return 0;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    total_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    total_len += len;
    
    *ciphertext_len = total_len;
    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

int decrypt_data(const unsigned char *ciphertext, size_t ciphertext_len,
                 const unsigned char *key, const unsigned char *iv,
                 unsigned char *plaintext, size_t *plaintext_len) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int total_len = 0;
    
    if (!(ctx = EVP_CIPHER_CTX_new())) return 0;
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    total_len = len;
    
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    total_len += len;
    
    *plaintext_len = total_len;
    plaintext[total_len] = '\0';
    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

int generate_random_bytes(unsigned char *buffer, size_t length) {
    return RAND_bytes(buffer, length) == 1;
}

int verify_master_password(const char *password, const unsigned char *salt,
                          const unsigned char *stored_hash) {
    unsigned char computed_hash[HASH_SIZE];
    
    if (!hash_password(password, salt, computed_hash, HASH_SIZE)) {
        return 0;
    }
    
    return memcmp(computed_hash, stored_hash, HASH_SIZE) == 0;
}
