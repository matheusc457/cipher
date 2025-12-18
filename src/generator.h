#ifndef GENERATOR_H
#define GENERATOR_H

#include <stddef.h>

// Password generation options
typedef struct {
    int length;
    int use_uppercase;
    int use_lowercase;
    int use_numbers;
    int use_symbols;
} PasswordOptions;

// Password strength levels
typedef enum {
    STRENGTH_WEAK,
    STRENGTH_MEDIUM,
    STRENGTH_STRONG,
    STRENGTH_VERY_STRONG
} PasswordStrength;

// Generate password with given options
int generate_password(char *buffer, size_t buffer_size, PasswordOptions options);

// Calculate password strength
PasswordStrength calculate_strength(const char *password);

// Get strength description
const char* get_strength_description(PasswordStrength strength);

// Get strength color
const char* get_strength_color(PasswordStrength strength);

#endif // GENERATOR_H
