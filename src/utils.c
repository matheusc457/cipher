#include "utils.h"
#include <ctype.h>

#ifdef _WIN32
    #include <conio.h>
    #define CLEAR_COMMAND "cls"
#else
    #include <termios.h>
    #include <unistd.h>
    #define CLEAR_COMMAND "clear"
#endif

void clear_screen(void) {
    system(CLEAR_COMMAND);
}

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_header(void) {
    printf("\n");
    printf(COLOR_CYAN);
    printf("╔══════════════════════════════════════╗\n");
    printf("║      CIPHER PASSWORD MANAGER         ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf(COLOR_RESET);
    printf("\n");
}

void print_success(const char *message) {
    printf(COLOR_GREEN "[SUCCESS] %s" COLOR_RESET "\n", message);
}

void print_error(const char *message) {
    printf(COLOR_RED "[ERROR] %s" COLOR_RESET "\n", message);
}

void print_info(const char *message) {
    printf(COLOR_BLUE "[INFO] %s" COLOR_RESET "\n", message);
}

void press_enter_to_continue(void) {
    printf("\nPress ENTER to continue...");
    clear_input_buffer();
    getchar();
}

int get_int_input(const char *prompt, int min, int max) {
    int value;
    int valid = 0;
    
    do {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            if (value >= min && value <= max) {
                valid = 1;
            } else {
                print_error("Invalid range! Please try again.");
            }
        } else {
            print_error("Invalid input! Please enter a number.");
        }
        clear_input_buffer();
    } while (!valid);
    
    return value;
}

void get_string_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        trim_whitespace(buffer);
    }
}

void get_password_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    
#ifdef _WIN32
    int i = 0;
    char ch;
    while (i < size - 1) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b' && i > 0) {
            i--;
            printf("\b \b");
        } else if (ch != '\b') {
            buffer[i++] = ch;
            printf("*");
        }
    }
    buffer[i] = '\0';
    printf("\n");
#else
    struct termios old_term, new_term;
    
    // Disable echo
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
    }
    
    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    printf("\n");
#endif
}

void trim_whitespace(char *str) {
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
}
