#ifndef PASSPHRASE_H
#define PASSPHRASE_H

#include <stddef.h>

#define WORDLIST_FILE "data/eff_large_wordlist.txt"
#define MAX_WORDLIST_SIZE 7776
#define MAX_WORD_LENGTH 32
#define MAX_PASSPHRASE_LENGTH 512

// Passphrase generation options
typedef struct {
    int word_count;           // Number of words (4-10)
    int capitalize;           // Capitalize first letter of each word
    int include_numbers;      // Add numbers between words
    int include_symbols;      // Add symbols between words
    char separator;           // Separator character (space, -, _, etc)
} PassphraseOptions;

// Passphrase strength information
typedef struct {
    double entropy_bits;
    long long possible_combinations;
    char strength_level[20];
} PassphraseStrength;

// Initialize passphrase generator (load wordlist)
int passphrase_init(void);

// Cleanup passphrase generator
void passphrase_cleanup(void);

// Generate passphrase with given
