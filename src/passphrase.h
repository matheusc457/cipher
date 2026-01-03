#ifndef PASSPHRASE_H
#define PASSPHRASE_H

#include <stdio.h>
#include <stdlib.h>

// Maximum number of words in wordlist
#define MAX_WORDS 7776
#define MAX_WORD_LENGTH 32

// Get wordlist path (returns path relative to executable or data directory)
const char* get_wordlist_path(void);

// Passphrase configuration structure
typedef struct {
    int num_words;           // Number of words (3-8)
    char separator;          // Separator: '-', '_', ' ', or '\0' for none
    int capitalize;          // 1 = capitalize first letter, 0 = lowercase
    int add_number;          // 1 = add random number at end, 0 = no number
    int add_symbols;         // 1 = add symbols between words (future feature)
} PassphraseConfig;

// Preset levels
typedef enum {
    PRESET_BASIC = 1,      // 3 words, 39 bits
    PRESET_STANDARD = 2,   // 4 words, 52 bits (recommended)
    PRESET_STRONG = 3,     // 5 words, 65 bits
    PRESET_MAXIMUM = 4,    // 6 words, 77 bits
    PRESET_CUSTOM = 5      // User-defined
} PresetLevel;

// Function declarations

// Initialize passphrase generator (load wordlist)
int passphrase_init(void);

// Clean up and free memory
void passphrase_cleanup(void);

// Generate passphrase with given configuration
char* generate_passphrase(PassphraseConfig *config);

// Get preset configuration
PassphraseConfig get_preset_config(PresetLevel level);

// Calculate entropy in bits
double calculate_entropy(int num_words);

// Display the passphrase generator menu
void display_passphrase_menu(void);

// Get custom configuration from user
PassphraseConfig get_custom_config(void);

// Display passphrase with formatting
void display_passphrase(const char *passphrase, PassphraseConfig *config);

// Check if wordlist is loaded
int is_wordlist_loaded(void);

#endif // PASSPHRASE_H
