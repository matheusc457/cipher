#ifndef PASSWORD_H
#define PASSWORD_H

#include <stddef.h>

#define MAX_SERVICE_NAME 100
#define MAX_USERNAME 100
#define MAX_PASSWORD 128

// Password entry structure
typedef struct {
    char service[MAX_SERVICE_NAME];
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} PasswordEntry;

// Password manager structure
typedef struct {
    PasswordEntry *entries;
    size_t count;
    size_t capacity;
} PasswordManager;

// Initialize password manager
PasswordManager* pm_init(void);

// Free password manager
void pm_free(PasswordManager *pm);

// Add new password entry
int pm_add_entry(PasswordManager *pm, const char *service, 
                 const char *username, const char *password);

// Find entry by service name
PasswordEntry* pm_find_entry(PasswordManager *pm, const char *service);

// Update existing entry
int pm_update_entry(PasswordManager *pm, const char *service,
                    const char *new_username, const char *new_password);

// Delete entry
int pm_delete_entry(PasswordManager *pm, const char *service);

// List all services
void pm_list_services(PasswordManager *pm);

// Get entry count
size_t pm_get_count(PasswordManager *pm);

// Check if service exists
int pm_service_exists(PasswordManager *pm, const char *service);

#endif // PASSWORD_H
