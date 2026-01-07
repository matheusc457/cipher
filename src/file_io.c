#include "file_io.h"
#include "crypto.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
#endif

static char data_dir_path[512] = {0};
static char data_file_path[512] = {0};
static char backup_file_path[512] = {0};

// Get or create the data directory
const char* get_data_dir(void) {
    if (data_dir_path[0] != '\0') {
        return data_dir_path;
    }
    
    const char *home = getenv("HOME");
    
#ifdef _WIN32
    if (!home) {
        home = getenv("USERPROFILE");
    }
#endif
    
    if (!home) {
        // Fallback to current directory
        strncpy(data_dir_path, ".cipher", sizeof(data_dir_path) - 1);
    } else {
        // Use ~/.cipher on Unix or %USERPROFILE%\.cipher on Windows
        snprintf(data_dir_path, sizeof(data_dir_path), "%s/.cipher", home);
    }
    
    return data_dir_path;
}

// Get full path to data file
static const char* get_data_file_path(void) {
    if (data_file_path[0] != '\0') {
        return data_file_path;
    }
    
    const char *dir = get_data_dir();
    snprintf(data_file_path, sizeof(data_file_path), "%s/%s", dir, DATA_FILE_NAME);
    return data_file_path;
}

// Get full path to backup file
static const char* get_backup_file_path(void) {
    if (backup_file_path[0] != '\0') {
        return backup_file_path;
    }
    
    const char *dir = get_data_dir();
    snprintf(backup_file_path, sizeof(backup_file_path), "%s/%s", dir, BACKUP_FILE_NAME);
    return backup_file_path;
}

int file_init(void) {
    const char *dir = get_data_dir();
    
    // Create directory if it doesn't exist
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
#ifdef _WIN32
        mkdir(dir);
#else
        mkdir(dir, 0700);
#endif
    }
    
    return 1;
}

int file_exists(void) {
    FILE *file = fopen(get_data_file_path(), "rb");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int file_save(PasswordManager *pm, const char *master_password) {
    if (!pm || !master_password) return 0;
    
    FILE *file = fopen(get_data_file_path(), "wb");
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
    
    // Handle empty vault (no entries)
    if (pm->count == 0) {
        // Write zero length for empty vault
        size_t ciphertext_len = 0;
        if (fwrite(&ciphertext_len, sizeof(size_t), 1, file) != 1) {
            fclose(file);
            return 0;
        }
        fclose(file);
        memset(key, 0, KEY_SIZE);
        return 1;
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
    
    FILE *file = fopen(get_data_file_path(), "rb");
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
    fclose(file);
    
    // Handle empty vault (no entries)
    if (ciphertext_len == 0 || header.entry_count == 0) {
        memset(key, 0, KEY_SIZE);
        
        // Create empty password manager
        PasswordManager *pm = pm_init();
        if (!pm) return NULL;
        
        *success = 1;
        return pm;
    }
    
    unsigned char *ciphertext = malloc(ciphertext_len);
    if (!ciphertext) {
        return NULL;
    }
    
    // Re-open file to read encrypted data
    file = fopen(get_data_file_path(), "rb");
    if (!file) {
        free(ciphertext);
        return NULL;
    }
    
    // Skip header and ciphertext_len
    fseek(file, sizeof(FileHeader) + sizeof(size_t), SEEK_SET);
    
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
    FILE *file = fopen(get_data_file_path(), "rb");
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
    FILE *src = fopen(get_data_file_path(), "rb");
    if (!src) return 0;
    
    FILE *dst = fopen(get_backup_file_path(), "wb");
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

// Dummy functions for compatibility
int file_lock_vault(void) {
    return 1;
}

void file_unlock_vault(void) {
    // Do nothing
}

int file_is_vault_locked(void) {
    return 0;
}
