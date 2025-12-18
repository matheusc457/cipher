#include "file_io.h"
#include "crypto.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int file_init(void) {
#ifdef _WIN32
    mkdir("data");
#else
    mkdir("data", 0700);
#endif
    return 1;
}

int file_exists(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int file_save(PasswordManager *pm, const char *master_password) {
    if (!pm || !master_password) return 0;
    
    FILE *file = fopen(DATA_FILE, "wb");
    if (!file) return 0;
    
    // Generate salt and IV
    FileHeader header;
    if (!generate_random_bytes(header.salt, sizeof(header.salt)) ||
        !generate_random_bytes(header.iv, sizeof(header.iv))) {
        fclose(file);
        return 0;
    }
    
    // Hash master password
    if (!hash_password(master_password, header.salt, 
                      header.hash, sizeof(header.hash))) {
        fclose(file);
        return 0;
    }
    
    header.entry_count = pm->count;
    
    // Write header
    if (fwrite(&header, sizeof(FileHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Derive encryption key
    unsigned char key[KEY_SIZE];
    if (!derive_key(master_password, header.salt, key, KEY_SIZE)) {
        fclose(file);
        return 0;
    }
    
    // Serialize and encrypt entries
    size_t data_size = sizeof(PasswordEntry) * pm->count;
    unsigned char *plaintext = (unsigned char*)pm->entries;
    unsigned char *ciphertext = malloc(data_size + 128); // Extra space for padding
    if (!ciphertext) {
        fclose(file);
        return 0;
    }
    
    size_t ciphertext_len;
    if (!encrypt_data(plaintext, data_size, key, header.iv, 
                     ciphertext, &ciphertext_len)) {
        free(ciphertext);
        fclose(file);
        return 0;
    }
    
    // Write encrypted data
    if (fwrite(&ciphertext_len, sizeof(size_t), 1, file) != 1 ||
        fwrite(ciphertext, 1, ciphertext_len, file) != ciphertext_len) {
        free(ciphertext);
        fclose(file);
        return 0;
    }
    
    free(ciphertext);
    fclose(file);
    
    // Clear sensitive data
    memset(key, 0, KEY_SIZE);
    
    return 1;
}

PasswordManager* file_load(const char *master_password, int *success) {
    *success = 0;
    
    FILE *file = fopen(DATA_FILE, "rb");
    if (!file) return NULL;
    
    // Read header
    FileHeader header;
    if (fread(&header, sizeof(FileHeader), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    // Verify master password
    if (!verify_master_password(master_password, header.salt, header.hash)) {
        fclose(file);
        return NULL;
    }
    
    // Derive decryption key
    unsigned char key[KEY_SIZE];
    if (!derive_key(master_password, header.salt, key, KEY_SIZE)) {
        fclose(file);
        return NULL;
    }
    
    // Read encrypted data
    size_t ciphertext_len;
    if (fread(&ciphertext_len, sizeof(size_t), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    unsigned char *ciphertext = malloc(ciphertext_len);
    if (!ciphertext) {
        fclose(file);
        return NULL;
    }
    
    if (fread(ciphertext, 1, ciphertext_len, file) != ciphertext_len) {
        free(ciphertext);
        fclose(file);
        return NULL;
    }
    fclose(file);
    
    // Decrypt data
    size_t plaintext_len;
    size_t data_size = sizeof(PasswordEntry) * header.entry_count;
    unsigned char *plaintext = malloc(data_size + 128);
    if (!plaintext) {
        free(ciphertext);
        return NULL;
    }
    
    if (!decrypt_data(ciphertext, ciphertext_len, key, header.iv,
                     plaintext, &plaintext_len)) {
        free(ciphertext);
        free(plaintext);
        return NULL;
    }
    
    free(ciphertext);
    memset(key, 0, KEY_SIZE);
    
    // Create password manager
    PasswordManager *pm = pm_init();
    if (!pm) {
        free(plaintext);
        return NULL;
    }
    
    // Copy decrypted entries
    pm->entries = realloc(pm->entries, data_size);
    if (!pm->entries) {
        pm_free(pm);
        free(plaintext);
        return NULL;
    }
    
    memcpy(pm->entries, plaintext, data_size);
    pm->count = header.entry_count;
    pm->capacity = header.entry_count;
    
    free(plaintext);
    *success = 1;
    return pm;
}

int file_verify_master_password(const char *master_password) {
    FILE *file = fopen(DATA_FILE, "rb");
    if (!file) return 0;
    
    FileHeader header;
    if (fread(&header, sizeof(FileHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    fclose(file);
    
    return verify_master_password(master_password, header.salt, header.hash);
}

int file_create_backup(void) {
    FILE *src = fopen(DATA_FILE, "rb");
    if (!src) return 0;
    
    FILE *dst = fopen(BACKUP_FILE, "wb");
    if (!dst) {
        fclose(src);
        return 0;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            fclose(src);
            fclose(dst);
            return 0;
        }
    }
    
    fclose(src);
    fclose(dst);
    return 1;
}

int file_change_master_password(PasswordManager *pm,
                                const char *old_password,
                                const char *new_password) {
    if (!file_verify_master_password(old_password)) {
        return 0;
    }
    
    if (!file_create_backup()) {
        return 0;
    }
    
    return file_save(pm, new_password);
}
