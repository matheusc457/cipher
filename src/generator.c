#include "generator.h"
#include "crypto.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>

#define UPPERCASE "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWERCASE "abcdefghijklmnopqrstuvwxyz"
#define NUMBERS "0123456789"
#define SYMBOLS "!@#$%^&*()_+-=[]{}|;:,.<>?"

int generate_password(char *buffer, size_t buffer_size, PasswordOptions options) {
    if (!buffer || buffer_size < (size_t)(options.length + 1)) return 0;  // Correção: Cast para size_t
    if (options.length < 4 || options.length > 128) return 0;
    
    char charset[256] = {0};
    int charset_len = 0;
    
    // Build character set
    if (options.use_uppercase) {
        strcat(charset, UPPERCASE);
        charset_len += strlen(UPPERCASE);
    }
    if (options.use_lowercase) {
        strcat(charset, LOWERCASE);
        charset_len += strlen(LOWERCASE);
    }
    if (options.use_numbers) {
        strcat(charset, NUMBERS);
        charset_len += strlen(NUMBERS);
    }
    if (options.use_symbols) {
        strcat(charset, SYMBOLS);
        charset_len += strlen(SYMBOLS);
    }
    
    if (charset_len == 0) return 0;
    
    // Generate random password
    unsigned char random_bytes[128];
    if (!generate_random_bytes(random_bytes, options.length)) {
        return 0;
    }
    
    for (int i = 0; i < options.length; i++) {
        buffer[i] = charset[random_bytes[i] % charset_len];
    }
    buffer[options.length] = '\0';
    
    return 1;
}

PasswordStrength calculate_strength(const char *password) {
    if (!password) return STRENGTH_WEAK;
    
    int len = strlen(password);
    int has_upper = 0, has_lower = 0, has_digit = 0, has_symbol = 0;
    
    for (int i = 0; i < len; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_symbol = 1;
    }
    
    int complexity = has_upper + has_lower + has_digit + has_symbol;
    
    // Score calculation
    int score = 0;
    if (len >= 8) score++;
    if (len >= 12) score++;
    if (len >= 16) score++;
    if (complexity >= 3) score++;
    if (complexity == 4) score++;
    
    if (score <= 1) return STRENGTH_WEAK;
    if (score <= 3) return STRENGTH_MEDIUM;
    if (score <= 4) return STRENGTH_STRONG;
    return STRENGTH_VERY_STRONG;
}

const char* get_strength_description(PasswordStrength strength) {
    switch (strength) {
        case STRENGTH_WEAK: return "WEAK";
        case STRENGTH_MEDIUM: return "MEDIUM";
        case STRENGTH_STRONG: return "STRONG";
        case STRENGTH_VERY_STRONG: return "VERY STRONG";
        default: return "UNKNOWN";
    }
}

const char* get_strength_color(PasswordStrength strength) {
    switch (strength) {
        case STRENGTH_WEAK: return COLOR_RED;
        case STRENGTH_MEDIUM: return COLOR_YELLOW;
        case STRENGTH_STRONG: return COLOR_GREEN;
        case STRENGTH_VERY_STRONG: return COLOR_CYAN;
        default: return COLOR_RESET;
    }
}
