#ifndef FILE_IO_H
#define FILE_IO_H

#include "password.h"

// Get the data directory path (creates if doesn't exist)
const char* get_data_dir(void);

// Data file paths (use get_data_dir() to construct full paths)
#define DATA_FILE_NAME "passwords.dat"
#define BACKUP_FILE_NAME "passwords.dat.backup"

// File header structure
typedef struct {
    unsigned char salt[16];
    unsigned char hash[32];
    unsigned char iv[16];
    size_t entry_count;
} FileHeader;

// Initialize data directory
int file_init(void);

// Check if password file exists
int file_exists(void);

// Save password manager to file
int file_save(PasswordManager *pm, const char *master_password);

// Load password manager from file
PasswordManager* file_load(const char *master_password, int *success);

// Verify master password from file
int file_verify_master_password(const char *master_password);

// Create backup of password file
int file_create_backup(void);

// Change master password
int file_change_master_password(PasswordManager *pm, 
                                const char *old_password,
                                const char *new_password);

#endif // FILE_IO_H
