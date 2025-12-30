#include "passphrase.h"
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <openssl/rand.h>

// Global wordlist storage
static char wordlist[MAX_WORDS][MAX_WORD_LENGTH];
static int wordlist_size = 0;
static int wordlist_loaded = 0;

// Get cryptographically secure random number
static unsigned int get_random_number(unsigned int max) {
    unsigned char buf[4];
    unsigned int num;
    
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        // Fallback to standard rand() if OpenSSL fails
        return rand() % max;
    }
    
    num = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return num % max;
}

// Load wordlist from file
int passphrase_init(void) {
    FILE *file = fopen(WORDLIST_PATH, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open wordlist file: %s\n", WORDLIST_PATH);
        fprintf(stderr, "Download it with:\n");
        fprintf(stderr, "  cd data && wget https://www.eff.org/files/2016/07/18/eff_large_wordlist.txt\n");
        return -1;
    }
    
    char line[256];
    wordlist_size = 0;
    
    while (fgets(line, sizeof(line), file) && wordlist_size < MAX_WORDS) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Parse line (format: "11111  abacus" or just "abacus")
        char *word = NULL;
        
        // Check if line starts with digits (EFF format)
        if (isdigit(line[0])) {
            // Find the word after the dice number
            word = strchr(line, '\t');
            if (!word) word = strchr(line, ' ');
            if (word) {
                // Skip whitespace
                while (*word && isspace(*word)) word++;
            }
        } else {
            word = line;
        }
        
        if (word && strlen(word) > 0 && strlen(word) < MAX_WORD_LENGTH) {
            strncpy(wordlist[wordlist_size], word, MAX_WORD_LENGTH - 1);
            wordlist[wordlist_size][MAX_WORD_LENGTH - 1] = '\0';
            wordlist_size++;
        }
    }
    
    fclose(file);
    
    if (wordlist_size < 1000) {
        fprintf(stderr, "Warning: Wordlist seems too small (%d words)\n", wordlist_size);
        fprintf(stderr, "Expected 7776 words for proper entropy.\n");
        return -1;
    }
    
    wordlist_loaded = 1;
    printf("Loaded %d words from wordlist\n", wordlist_size);
    return 0;
}

// Clean up
void passphrase_cleanup(void) {
    wordlist_loaded = 0;
    wordlist_size = 0;
}

// Check if wordlist is loaded
int is_wordlist_loaded(void) {
    return wordlist_loaded;
}

// Capitalize first letter of a word
static void capitalize_word(char *word) {
    if (word && word[0]) {
        word[0] = toupper(word[0]);
    }
}

// Generate passphrase
char* generate_passphrase(PassphraseConfig *config) {
    if (!wordlist_loaded || wordlist_size == 0) {
        fprintf(stderr, "Error: Wordlist not loaded!\n");
        return NULL;
    }
    
    // Allocate memory for passphrase
    char *passphrase = malloc(512);
    if (!passphrase) return NULL;
    
    passphrase[0] = '\0';
    
    // Generate words
    for (int i = 0; i < config->num_words; i++) {
        // Get random word index
        int index = get_random_number(wordlist_size);
        char word[MAX_WORD_LENGTH];
        strncpy(word, wordlist[index], MAX_WORD_LENGTH);
        
        // Capitalize if needed
        if (config->capitalize) {
            capitalize_word(word);
        }
        
        // Add word to passphrase
        strcat(passphrase, word);
        
        // Add separator if not last word
        if (i < config->num_words - 1 && config->separator != '\0') {
            char sep[2] = {config->separator, '\0'};
            strcat(passphrase, sep);
        }
    }
    
    // Add random number if requested
    if (config->add_number) {
        char num_str[16];
        unsigned int random_num = get_random_number(10000);
        sprintf(num_str, "%c%04u", 
                config->separator != '\0' ? config->separator : '-', 
                random_num);
        strcat(passphrase, num_str);
    }
    
    return passphrase;
}

// Get preset configuration
PassphraseConfig get_preset_config(PresetLevel level) {
    PassphraseConfig config;
    config.separator = '-';
    config.capitalize = 0;
    config.add_number = 0;
    config.add_symbols = 0;
    
    switch (level) {
        case PRESET_BASIC:
            config.num_words = 3;
            break;
        case PRESET_STANDARD:
            config.num_words = 4;
            break;
        case PRESET_STRONG:
            config.num_words = 5;
            break;
        case PRESET_MAXIMUM:
            config.num_words = 6;
            break;
        default:
            config.num_words = 4;
    }
    
    return config;
}

// Calculate entropy
double calculate_entropy(int num_words) {
    if (wordlist_size == 0) return 0.0;
    return num_words * log2((double)wordlist_size);
}

// Get crack time estimate string
static void get_crack_time(double entropy, char *buffer, size_t size) {
    // Assume 100 billion guesses per second
    double seconds = pow(2, entropy) / 100000000000.0;
    
    if (seconds < 60) {
        snprintf(buffer, size, "~%.0f seconds", seconds);
    } else if (seconds < 3600) {
        snprintf(buffer, size, "~%.0f minutes", seconds / 60);
    } else if (seconds < 86400) {
        snprintf(buffer, size, "~%.0f hours", seconds / 3600);
    } else if (seconds < 31536000) {
        snprintf(buffer, size, "~%.0f days", seconds / 86400);
    } else if (seconds < 31536000000.0) {
        snprintf(buffer, size, "~%.0f years", seconds / 31536000);
    } else if (seconds < 31536000000000.0) {
        snprintf(buffer, size, "~%.0fk years", seconds / 31536000000.0);
    } else {
        snprintf(buffer, size, "~%.0fM years", seconds / 31536000000000.0);
    }
}

// Get custom configuration from user
PassphraseConfig get_custom_config(void) {
    PassphraseConfig config;
    char input[10];
    
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║    CUSTOM PASSPHRASE                 ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    // Number of words
    printf("Number of words (3-8): ");
    fgets(input, sizeof(input), stdin);
    config.num_words = atoi(input);
    if (config.num_words < 3) config.num_words = 3;
    if (config.num_words > 8) config.num_words = 8;
    
    // Separator
    printf("\nSeparator:\n");
    printf("[1] Hyphen (-)      → word-word-word\n");
    printf("[2] Underscore (_)  → word_word_word\n");
    printf("[3] Space ( )       → word word word\n");
    printf("[4] None            → wordwordword\n");
    printf("Choice: ");
    fgets(input, sizeof(input), stdin);
    int sep_choice = atoi(input);
    
    switch (sep_choice) {
        case 1: config.separator = '-'; break;
        case 2: config.separator = '_'; break;
        case 3: config.separator = ' '; break;
        case 4: config.separator = '\0'; break;
        default: config.separator = '-';
    }
    
    // Capitalize
    printf("\nCapitalize first letter of each word? (y/n): ");
    fgets(input, sizeof(input), stdin);
    config.capitalize = (input[0] == 'y' || input[0] == 'Y') ? 1 : 0;
    
    // Add number
    printf("Add random number at the end? (y/n): ");
    fgets(input, sizeof(input), stdin);
    config.add_number = (input[0] == 'y' || input[0] == 'Y') ? 1 : 0;
    
    config.add_symbols = 0; // Future feature
    
    return config;
}

// Display passphrase with formatting
void display_passphrase(const char *passphrase, PassphraseConfig *config) {
    double entropy = calculate_entropy(config->num_words);
    char crack_time[64];
    get_crack_time(entropy, crack_time, sizeof(crack_time));
    
    printf("\n┌─────────────────────────────────────┐\n");
    printf("│  %-35s│\n", passphrase);
    printf("└─────────────────────────────────────┘\n\n");
    
    // Strength bar
    int strength_bars = (int)(entropy / 10);
    if (strength_bars > 10) strength_bars = 10;
    
    printf("Strength: ");
    for (int i = 0; i < strength_bars; i++) printf("█");
    for (int i = strength_bars; i < 10; i++) printf("░");
    
    if (entropy < 40) printf(" WEAK\n");
    else if (entropy < 60) printf(" STRONG\n");
    else printf(" VERY STRONG\n");
    
    printf("Entropy: %.1f bits\n", entropy);
    printf("Crack time: %s\n", crack_time);
}

// Display passphrase menu
void display_passphrase_menu(void) {
    if (!wordlist_loaded) {
        printf("\nInitializing passphrase generator...\n");
        if (passphrase_init() != 0) {
            printf("\nFailed to initialize passphrase generator.\n");
            printf("Press Enter to return to main menu...");
            getchar();
            return;
        }
    }
    
    while (1) {
        printf("\n╔══════════════════════════════════════╗\n");
        printf("║    PASSPHRASE GENERATOR              ║\n");
        printf("╚══════════════════════════════════════╝\n\n");
        
        printf("Choose strength level:\n\n");
        printf("[1] Basic    - 3 words (%.0f bits)\n", calculate_entropy(3));
        printf("    Example: apple-tree-cloud\n");
        printf("    Crack time: ~1 year\n\n");
        
        printf("[2] Standard - 4 words (%.0f bits)\n", calculate_entropy(4));
        printf("    Example: apple-tree-cloud-river\n");
        printf("    Crack time: ~71 years\n\n");
        
        printf("[3] Strong   - 5 words (%.0f bits)\n", calculate_entropy(5));
        printf("    Example: apple-tree-cloud-river-moon\n");
        printf("    Crack time: ~584k years\n\n");
        
        printf("[4] Maximum  - 6 words (%.0f bits)\n", calculate_entropy(6));
        printf("    Example: apple-tree-cloud-river-moon-star\n");
        printf("    Crack time: ~4.7M years\n\n");
        
        printf("[5] Custom   - Configure manually\n\n");
        printf("[0] Back to main menu\n\n");
        
        printf("Choice: ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        int choice = atoi(input);
        
        if (choice == 0) {
            break;
        }
        
        PassphraseConfig config;
        
        if (choice >= 1 && choice <= 4) {
            config = get_preset_config((PresetLevel)choice);
        } else if (choice == 5) {
            config = get_custom_config();
        } else {
            printf("Invalid choice!\n");
            continue;
        }
        
        printf("\nGenerating passphrase...\n");
        char *passphrase = generate_passphrase(&config);
        
        if (passphrase) {
            display_passphrase(passphrase, &config);
            
            // Sub-menu
            while (1) {
                printf("\nWhat would you like to do?\n");
                printf("[1] Generate another\n");
                printf("[2] Copy to clipboard (manual)\n");
                printf("[3] Back\n\n");
                printf("Choice: ");
                
                fgets(input, sizeof(input), stdin);
                int sub_choice = atoi(input);
                
                if (sub_choice == 1) {
                    free(passphrase);
                    passphrase = generate_passphrase(&config);
                    if (passphrase) {
                        display_passphrase(passphrase, &config);
                    }
                } else if (sub_choice == 2) {
                    printf("\nPassphrase: %s\n", passphrase);
                    printf("(Copy it manually - clipboard integration coming soon!)\n");
                } else {
                    break;
                }
            }
            
            free(passphrase);
        }
    }
}
