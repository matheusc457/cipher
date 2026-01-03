#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Function prototypes
void clear_screen(void);
void clear_input_buffer(void);
void print_header(void);
void print_success(const char *message);
void print_error(const char *message);
void print_info(const char *format, ...);
void press_enter_to_continue(void);
int get_int_input(const char *prompt, int min, int max);
void get_string_input(const char *prompt, char *buffer, size_t size);
void get_password_input(const char *prompt, char *buffer, size_t size);
void trim_whitespace(char *str);

// Password display (show/hide)
void print_password_hidden(const char *password, int show);

#endif // UTILS_H
