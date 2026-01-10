#ifndef _WIN32
    #define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <unistd.h>
#endif

#include "utils.h"
#include "crypto.h"
#include "password.h"
#include "generator.h"
#include "passphrase.h"
#include "clipboard.h"
#include "file_io.h"

#define MASTER_PASSWORD_SIZE 256

static PasswordManager *pm = NULL;
static char master_password[MASTER_PASSWORD_SIZE];

// Signal handler for cleanup on Ctrl+C or termination
static void signal_handler(int signum) {
    (void)signum; // Unused parameter
    
    printf("\n\n");
    print_info("Received interrupt signal. Cleaning up...");
    
    // Cleanup
    if (pm) {
        memset(master_password, 0, sizeof(master_password));
        pm_free(pm);
    }
    passphrase_cleanup();
    crypto_cleanup();
    file_unlock_vault();
    
    printf("\nThank you for using Cipher!\n\n");
    exit(0);
}

// Setup signal handlers
static void setup_signal_handlers(void) {
#ifdef _WIN32
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#else
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);  // Ctrl+C
    sigaction(SIGTERM, &sa, NULL); // kill command
    sigaction(SIGHUP, &sa, NULL);  // Terminal closed
#endif
}

void show_menu(void) {
    printf("\n");
    printf(COLOR_CYAN "╔════════════════════════════════════════╗\n");
    printf("║             MAIN MENU                  ║\n");
    printf("╚════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
    printf("  [1] Add new password\n");
    printf("  [2] Search password\n");
    printf("  [3] List all services\n");
    printf("  [4] Update password\n");
    printf("  [5] Delete password\n");
    printf("  [6] Generate strong password\n");
    printf("  [7] Generate passphrase\n");
    printf("  [8] Change master password\n");
    printf("  [9] Save and exit\n");
    printf("\n");
}

void add_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Add New Password ===\n\n" COLOR_RESET);
    
    char service[MAX_SERVICE_NAME];
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char choice;
    
    get_string_input("Service name: ", service, sizeof(service));
    if (strlen(service) == 0) {
        print_error("Service name cannot be empty!");
        press_enter_to_continue();
        return;
    }
    
    if (pm_service_exists(pm, service)) {
        print_error("Service already exists! Use update instead.");
        press_enter_to_continue();
        return;
    }
    
    get_string_input("Username/Email: ", username, sizeof(username));
    if (strlen(username) == 0) {
        print_error("Username cannot be empty!");
        press_enter_to_continue();
        return;
    }
    
    printf("\nPassword options:\n");
    printf("  [1] Enter manually\n");
    printf("  [2] Generate random password\n");
    printf("  [3] Generate passphrase\n");
    choice = get_int_input("Choose: ", 1, 3);
    
    if (choice == 2) {
        PasswordOptions opts;
        opts.length = get_int_input("\nPassword length (8-32): ", 8, 32);
        
        char yesno[10];
        get_string_input("Include uppercase? (y/n): ", yesno, sizeof(yesno));
        opts.use_uppercase = (yesno[0] == 'y' || yesno[0] == 'Y');
        
        get_string_input("Include lowercase? (y/n): ", yesno, sizeof(yesno));
        opts.use_lowercase = (yesno[0] == 'y' || yesno[0] == 'Y');
        
        get_string_input("Include numbers? (y/n): ", yesno, sizeof(yesno));
        opts.use_numbers = (yesno[0] == 'y' || yesno[0] == 'Y');
        
        get_string_input("Include symbols? (y/n): ", yesno, sizeof(yesno));
        opts.use_symbols = (yesno[0] == 'y' || yesno[0] == 'Y');
        
        if (generate_password(password, sizeof(password), opts)) {
            printf("\nGenerated password: %s%s%s\n", 
                   COLOR_GREEN, password, COLOR_RESET);
            
            PasswordStrength strength = calculate_strength(password);
            printf("Strength: %s%s%s\n",
                   get_strength_color(strength),
                   get_strength_description(strength),
                   COLOR_RESET);
            
            // Clipboard integration
            if (clipboard_is_available()) {
                if (clipboard_copy_with_timeout(password, 45)) {
                    print_info("Password copied to clipboard! (auto-clears in 45s)");
                }
            }
        } else {
            print_error("Failed to generate password!");
            press_enter_to_continue();
            return;
        }
    } else if (choice == 3) {
        // Initialize passphrase generator if needed
        if (!is_wordlist_loaded()) {
            print_info("Loading wordlist...");
            if (passphrase_init() != 0) {
                print_error("Failed to load wordlist!");
                press_enter_to_continue();
                return;
            }
        }
        
        printf("\nPassphrase presets:\n");
        printf("  [1] Basic    - 3 words\n");
        printf("  [2] Standard - 4 words (recommended)\n");
        printf("  [3] Strong   - 5 words\n");
        int preset = get_int_input("Choose preset: ", 1, 3);
        
        PassphraseConfig config = get_preset_config((PresetLevel)preset);
        char *generated = generate_passphrase(&config);
        
        if (generated) {
            strncpy(password, generated, sizeof(password) - 1);
            password[sizeof(password) - 1] = '\0';
            
            printf("\nGenerated passphrase: %s%s%s\n", 
                   COLOR_GREEN, password, COLOR_RESET);
            
            double entropy = calculate_entropy(config.num_words);
            printf("Entropy: %.1f bits\n", entropy);
            
            // Clipboard integration
            if (clipboard_is_available()) {
                if (clipboard_copy_with_timeout(password, 45)) {
                    print_info("Passphrase copied to clipboard! (auto-clears in 45s)");
                }
            }
            
            free(generated);
        } else {
            print_error("Failed to generate passphrase!");
            press_enter_to_continue();
            return;
        }
    } else {
        get_password_input("\nPassword: ", password, sizeof(password));
        if (strlen(password) == 0) {
            print_error("Password cannot be empty!");
            press_enter_to_continue();
            return;
        }
    }
    
    if (pm_add_entry(pm, service, username, password)) {
        print_success("Password added successfully!");
    } else {
        print_error("Failed to add password!");
    }
    
    // Clear sensitive data
    memset(password, 0, sizeof(password));
    press_enter_to_continue();
}

void search_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Search Password ===\n\n" COLOR_RESET);
    
    char service[MAX_SERVICE_NAME];
    get_string_input("Service name: ", service, sizeof(service));
    
    PasswordEntry *entry = pm_find_entry(pm, service);
    if (!entry) {
        print_error("Service not found!");
        press_enter_to_continue();
        return;
    }
    
    // Interactive menu with show/hide/copy
    int show_password = 0;
    int running = 1;
    
    while (running) {
        clear_screen();
        print_header();
        
        printf(COLOR_CYAN "╔════════════════════════════════════════╗\n");
        printf("║           Password Found               ║\n");
        printf("╚════════════════════════════════════════╝\n" COLOR_RESET);
        printf("\n");
        printf("  Service:  %s%s%s\n", COLOR_GREEN, entry->service, COLOR_RESET);
        printf("  Username: %s\n", entry->username);
        printf("  Password: ");
        print_password_hidden(entry->password, show_password);
        printf("\n\n");
        
        PasswordStrength strength = calculate_strength(entry->password);
        printf("  Strength: %s%s%s\n",
               get_strength_color(strength),
               get_strength_description(strength),
               COLOR_RESET);
        
        printf("\n");
        printf(COLOR_CYAN "Actions:\n" COLOR_RESET);
        printf("  [S] %s password\n", show_password ? "Hide" : "Show");
        
        if (clipboard_is_available()) {
            printf("  [C] Copy to clipboard (auto-clears in 30s)\n");
        }
        
        printf("  [B] Back to menu\n");
        printf("\n");
        
        char choice[10];
        get_string_input("Choose: ", choice, sizeof(choice));
        
        if (choice[0] == 's' || choice[0] == 'S') {
            show_password = !show_password;
        } else if ((choice[0] == 'c' || choice[0] == 'C') && clipboard_is_available()) {
            if (clipboard_copy_with_timeout(entry->password, 30)) {
                print_success("Password copied! Auto-clears in 30 seconds.");
                sleep(2);
            } else {
                print_error("Failed to copy to clipboard.");
                sleep(2);
            }
        } else if (choice[0] == 'b' || choice[0] == 'B') {
            running = 0;
        }
    }
}

void update_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Update Password ===\n\n" COLOR_RESET);
    
    char service[MAX_SERVICE_NAME];
    get_string_input("Service name: ", service, sizeof(service));
    
    if (!pm_service_exists(pm, service)) {
        print_error("Service not found!");
        press_enter_to_continue();
        return;
    }
    
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    
    printf("\nLeave empty to keep current value\n");
    get_string_input("New username (or ENTER to skip): ", username, sizeof(username));
    get_password_input("New password (or ENTER to skip): ", password, sizeof(password));
    
    if (strlen(username) == 0 && strlen(password) == 0) {
        print_info("No changes made.");
        press_enter_to_continue();
        return;
    }
    
    if (pm_update_entry(pm, service,
                       strlen(username) > 0 ? username : NULL,
                       strlen(password) > 0 ? password : NULL)) {
        print_success("Password updated successfully!");
    } else {
        print_error("Failed to update password!");
    }
    
    memset(password, 0, sizeof(password));
    press_enter_to_continue();
}

void delete_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Delete Password ===\n\n" COLOR_RESET);
    
    char service[MAX_SERVICE_NAME];
    get_string_input("Service name: ", service, sizeof(service));
    
    if (!pm_service_exists(pm, service)) {
        print_error("Service not found!");
        press_enter_to_continue();
        return;
    }
    
    char confirm[10];
    get_string_input("Are you sure? (yes/no): ", confirm, sizeof(confirm));
    
    if (strcmp(confirm, "yes") == 0) {
        if (pm_delete_entry(pm, service)) {
            print_success("Password deleted successfully!");
        } else {
            print_error("Failed to delete password!");
        }
    } else {
        print_info("Deletion cancelled.");
    }
    
    press_enter_to_continue();
}

void generate_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Generate Strong Password ===\n\n" COLOR_RESET);
    
    PasswordOptions opts;
    opts.length = get_int_input("Password length (8-32): ", 8, 32);
    
    char yesno[10];
    get_string_input("Include uppercase? (y/n): ", yesno, sizeof(yesno));
    opts.use_uppercase = (yesno[0] == 'y' || yesno[0] == 'Y');
    
    get_string_input("Include lowercase? (y/n): ", yesno, sizeof(yesno));
    opts.use_lowercase = (yesno[0] == 'y' || yesno[0] == 'Y');
    
    get_string_input("Include numbers? (y/n): ", yesno, sizeof(yesno));
    opts.use_numbers = (yesno[0] == 'y' || yesno[0] == 'Y');
    
    get_string_input("Include symbols? (y/n): ", yesno, sizeof(yesno));
    opts.use_symbols = (yesno[0] == 'y' || yesno[0] == 'Y');
    
    char password[MAX_PASSWORD];
    if (generate_password(password, sizeof(password), opts)) {
        printf("\n");
        printf("Generated password: %s%s%s\n", 
               COLOR_GREEN, password, COLOR_RESET);
        
        PasswordStrength strength = calculate_strength(password);
        printf("Strength: %s%s%s\n",
               get_strength_color(strength),
               get_strength_description(strength),
               COLOR_RESET);
        
        // Clipboard integration
        if (clipboard_is_available()) {
            char copy[10];
            get_string_input("\nCopy to clipboard? (y/n): ", copy, sizeof(copy));
            if (copy[0] == 'y' || copy[0] == 'Y') {
                if (clipboard_copy_with_timeout(password, 30)) {
                    print_success("Password copied! Auto-clears in 30 seconds.");
                } else {
                    print_error("Failed to copy to clipboard.");
                }
            }
        }
    } else {
        print_error("Failed to generate password!");
    }
    
    memset(password, 0, sizeof(password));
    press_enter_to_continue();
}

void change_master_password_menu(void) {
    clear_screen();
    print_header();
    printf(COLOR_YELLOW "=== Change Master Password ===\n\n" COLOR_RESET);
    
    char old_pass[MASTER_PASSWORD_SIZE];
    char new_pass[MASTER_PASSWORD_SIZE];
    char confirm_pass[MASTER_PASSWORD_SIZE];
    
    get_password_input("Current master password: ", old_pass, sizeof(old_pass));
    
    if (strcmp(old_pass, master_password) != 0) {
        print_error("Incorrect password!");
        memset(old_pass, 0, sizeof(old_pass));
        press_enter_to_continue();
        return;
    }
    
    get_password_input("New master password: ", new_pass, sizeof(new_pass));
    get_password_input("Confirm new password: ", confirm_pass, sizeof(confirm_pass));
    
    if (strcmp(new_pass, confirm_pass) != 0) {
        print_error("Passwords don't match!");
    } else if (strlen(new_pass) < 8) {
        print_error("Password must be at least 8 characters!");
    } else {
        if (file_change_master_password(pm, old_pass, new_pass)) {
            strncpy(master_password, new_pass, sizeof(master_password) - 1);
            print_success("Master password changed successfully!");
        } else {
            print_error("Failed to change master password!");
        }
    }
    
    memset(old_pass, 0, sizeof(old_pass));
    memset(new_pass, 0, sizeof(new_pass));
    memset(confirm_pass, 0, sizeof(confirm_pass));
    press_enter_to_continue();
}

int setup_master_password(void) {
    clear_screen();
    print_header();
    print_info("No password vault found. Creating new vault...");
    printf("\n");
    
    char password[MASTER_PASSWORD_SIZE];
    char confirm[MASTER_PASSWORD_SIZE];
    
    get_password_input("Create master password: ", password, sizeof(password));
    
    if (strlen(password) < 8) {
        print_error("Master password must be at least 8 characters!");
        return 0;
    }
    
    get_password_input("Confirm master password: ", confirm, sizeof(confirm));
    
    if (strcmp(password, confirm) != 0) {
        print_error("Passwords don't match!");
        memset(password, 0, sizeof(password));
        memset(confirm, 0, sizeof(confirm));
        return 0;
    }
    
    strncpy(master_password, password, sizeof(master_password) - 1);
    memset(password, 0, sizeof(password));
    memset(confirm, 0, sizeof(confirm));
    
    pm = pm_init();
    if (!pm) {
        print_error("Failed to initialize password manager!");
        return 0;
    }
    
    if (!file_save(pm, master_password)) {
        print_error("Failed to create vault!");
        pm_free(pm);
        return 0;
    }
    
    print_success("Vault created successfully!");
    press_enter_to_continue();
    return 1;
}

int unlock_vault(void) {
    clear_screen();
    print_header();
    printf("\n");
    
    char password[MASTER_PASSWORD_SIZE];
    get_password_input("Enter master password: ", password, sizeof(password));
    
    int success;
    pm = file_load(password, &success);
    
    if (!success || !pm) {
        print_error("Incorrect password or corrupted vault!");
        memset(password, 0, sizeof(password));
        return 0;
    }
    
    strncpy(master_password, password, sizeof(master_password) - 1);
    memset(password, 0, sizeof(password));
    
    print_success("Vault unlocked successfully!");
    printf("\n");
    print_info("Loaded %zu password(s)", pm_get_count(pm));
    
    // Show clipboard status
    if (clipboard_is_available()) {
        print_info("Clipboard support: %s", clipboard_get_backend());
    } else {
        print_info("Clipboard not available. Install xclip/xsel/wl-clipboard for copy support.");
    }
    
    press_enter_to_continue();
    return 1;
}

int main(void) {
    // Setup signal handlers for cleanup
    setup_signal_handlers();
    
    // Initialize
    crypto_init();
    file_init();
    
    // Setup or unlock vault
    if (!file_exists()) {
        if (!setup_master_password()) {
            crypto_cleanup();
            return 1;
        }
    } else {
        if (!unlock_vault()) {
            crypto_cleanup();
            return 1;
        }
    }
    
    // Main loop
    int running = 1;
    while (running) {
        clear_screen();
        print_header();
        show_menu();
        
        int choice = get_int_input("Choose an option: ", 1, 9);
        
        switch (choice) {
            case 1: add_password_menu(); break;
            case 2: search_password_menu(); break;
            case 3:
                clear_screen();
                print_header();
                pm_list_services(pm);
                press_enter_to_continue();
                break;
            case 4: update_password_menu(); break;
            case 5: delete_password_menu(); break;
            case 6: generate_password_menu(); break;
            case 7:
                display_passphrase_menu();
                break;
            case 8: change_master_password_menu(); break;
            case 9:
                clear_screen();
                print_header();
                print_info("Saving vault...");
                if (file_save(pm, master_password)) {
                    print_success("Vault saved successfully!");
                } else {
                    print_error("Failed to save vault!");
                }
                running = 0;
                break;
        }
    }
    
    // Cleanup
    memset(master_password, 0, sizeof(master_password));
    pm_free(pm);
    passphrase_cleanup();
    crypto_cleanup();
    file_unlock_vault();
    
    printf("\nThank you for using Cipher!\n\n");
    return 0;
}
