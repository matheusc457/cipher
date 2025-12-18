#include "password.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h> // Necessário para strcasecmp

#define INITIAL_CAPACITY 10

PasswordManager* pm_init(void) {
    PasswordManager *pm = malloc(sizeof(PasswordManager));
    if (!pm) return NULL;
    
    pm->entries = malloc(sizeof(PasswordEntry) * INITIAL_CAPACITY);
    if (!pm->entries) {
        free(pm);
        return NULL;
    }
    
    pm->count = 0;
    pm->capacity = INITIAL_CAPACITY;
    return pm;
}

void pm_free(PasswordManager *pm) {
    if (!pm) return;
    
    // Clear sensitive data
    if (pm->entries) {
        memset(pm->entries, 0, sizeof(PasswordEntry) * pm->capacity);
        free(pm->entries);
    }
    
    free(pm);
}

static int pm_resize(PasswordManager *pm) {
    size_t new_capacity = pm->capacity * 2;
    PasswordEntry *new_entries = realloc(pm->entries, 
                                        sizeof(PasswordEntry) * new_capacity);
    if (!new_entries) return 0;
    
    pm->entries = new_entries;
    pm->capacity = new_capacity;
    return 1;
}

int pm_add_entry(PasswordManager *pm, const char *service,
                 const char *username, const char *password) {
    if (!pm || !service || !username || !password) return 0;
    
    // Check if service already exists
    if (pm_service_exists(pm, service)) {
        return 0;
    }
    
    // Resize if needed
    if (pm->count >= pm->capacity) {
        if (!pm_resize(pm)) return 0;
    }
    
    // Add new entry
    PasswordEntry *entry = &pm->entries[pm->count];
    strncpy(entry->service, service, MAX_SERVICE_NAME - 1);
    entry->service[MAX_SERVICE_NAME - 1] = '\0';
    
    strncpy(entry->username, username, MAX_USERNAME - 1);
    entry->username[MAX_USERNAME - 1] = '\0';
    
    strncpy(entry->password, password, MAX_PASSWORD - 1);
    entry->password[MAX_PASSWORD - 1] = '\0';
    
    pm->count++;
    return 1;
}

PasswordEntry* pm_find_entry(PasswordManager *pm, const char *service) {
    if (!pm || !service) return NULL;
    
    for (size_t i = 0; i < pm->count; i++) {
        if (strcasecmp(pm->entries[i].service, service) == 0) {
            return &pm->entries[i];
        }
    }
    
    return NULL;
}

int pm_update_entry(PasswordManager *pm, const char *service,
                    const char *new_username, const char *new_password) {
    PasswordEntry *entry = pm_find_entry(pm, service);
    if (!entry) return 0;
    
    if (new_username) {
        strncpy(entry->username, new_username, MAX_USERNAME - 1);
        entry->username[MAX_USERNAME - 1] = '\0';
    }
    
    if (new_password) {
        strncpy(entry->password, new_password, MAX_PASSWORD - 1);
        entry->password[MAX_PASSWORD - 1] = '\0';
    }
    
    return 1;
}

int pm_delete_entry(PasswordManager *pm, const char *service) {
    if (!pm || !service) return 0;
    
    for (size_t i = 0; i < pm->count; i++) {
        if (strcasecmp(pm->entries[i].service, service) == 0) {
            // Clear sensitive data
            memset(&pm->entries[i], 0, sizeof(PasswordEntry));
            
            // Shift remaining entries
            if (i < pm->count - 1) {
                memmove(&pm->entries[i], &pm->entries[i + 1],
                       sizeof(PasswordEntry) * (pm->count - i - 1));
            }
            
            pm->count--;
            return 1;
        }
    }
    
    return 0;
}

void pm_list_services(PasswordManager *pm) {
    if (!pm || pm->count == 0) {
        print_info("No passwords stored yet.");
        return;
    }
    
    printf("\n");
    printf(COLOR_CYAN "═══════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_CYAN "  Stored Services (%zu)\n" COLOR_RESET, pm->count);
    printf(COLOR_CYAN "═══════════════════════════════════════\n" COLOR_RESET);
    
    for (size_t i = 0; i < pm->count; i++) {
        printf("  %zu. %s%s%s\n", i + 1, 
               COLOR_GREEN, pm->entries[i].service, COLOR_RESET);
        printf("     └─ User: %s\n", pm->entries[i].username);
    }
    printf("\n");
}

size_t pm_get_count(PasswordManager *pm) {
    return pm ? pm->count : 0;
}

int pm_service_exists(PasswordManager *pm, const char *service) {
    return pm_find_entry(pm, service) != NULL;
}
