#include "clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

static char backend_name[32] = "unknown";

// Escape single quotes for shell commands
static void escape_for_shell(const char *input, char *output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < output_size - 1; i++) {
        if (input[i] == '\'') {
            if (j + 4 < output_size) {
                output[j++] = '\'';
                output[j++] = '\\';
                output[j++] = '\'';
                output[j++] = '\'';
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

#ifdef _WIN32
// Windows implementation using native API
int clipboard_copy(const char *text) {
    if (!text || strlen(text) == 0) return 0;
    
    if (!OpenClipboard(NULL)) return 0;
    
    EmptyClipboard();
    
    size_t len = strlen(text) + 1;
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hg) {
        CloseClipboard();
        return 0;
    }
    
    char *locked = (char*)GlobalLock(hg);
    if (!locked) {
        GlobalFree(hg);
        CloseClipboard();
        return 0;
    }
    
    memcpy(locked, text, len);
    GlobalUnlock(hg);
    
    if (!SetClipboardData(CF_TEXT, hg)) {
        GlobalFree(hg);
        CloseClipboard();
        return 0;
    }
    
    CloseClipboard();
    strncpy(backend_name, "native", sizeof(backend_name) - 1);
    return 1;
}

int clipboard_clear(void) {
    return clipboard_copy("");
}

DWORD WINAPI clear_clipboard_thread(LPVOID lpParam) {
    int seconds = *(int*)lpParam;
    Sleep(seconds * 1000);
    clipboard_clear();
    free(lpParam);
    return 0;
}

int clipboard_copy_with_timeout(const char *text, int seconds) {
    if (!clipboard_copy(text)) return 0;
    
    int *timeout_ptr = malloc(sizeof(int));
    if (!timeout_ptr) return 0;
    *timeout_ptr = seconds;
    
    HANDLE thread = CreateThread(NULL, 0, clear_clipboard_thread, 
                                 timeout_ptr, 0, NULL);
    if (thread) {
        CloseHandle(thread);
        return 1;
    }
    
    free(timeout_ptr);
    return 0;
}

int clipboard_is_available(void) {
    return 1; // Windows always has clipboard
}

#else
// Unix/Linux/macOS implementation using external commands

static int check_command(const char *command) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "which %s > /dev/null 2>&1", command);
    return system(cmd) == 0;
}

int clipboard_copy(const char *text) {
    if (!text || strlen(text) == 0) return 0;
    
    char escaped[8192];
    escape_for_shell(text, escaped, sizeof(escaped));
    
    char cmd[8192];
    
    // Try Wayland first (wl-copy)
    if (check_command("wl-copy")) {
        snprintf(cmd, sizeof(cmd), "printf '%%s' '%s' | wl-copy", escaped);
        if (system(cmd) == 0) {
            strncpy(backend_name, "wl-copy", sizeof(backend_name) - 1);
            return 1;
        }
    }
    
    // Try macOS (pbcopy)
    if (check_command("pbcopy")) {
        snprintf(cmd, sizeof(cmd), "printf '%%s' '%s' | pbcopy", escaped);
        if (system(cmd) == 0) {
            strncpy(backend_name, "pbcopy", sizeof(backend_name) - 1);
            return 1;
        }
    }
    
    // Try X11 - xclip
    if (check_command("xclip")) {
        snprintf(cmd, sizeof(cmd), 
                 "printf '%%s' '%s' | xclip -selection clipboard", escaped);
        if (system(cmd) == 0) {
            strncpy(backend_name, "xclip", sizeof(backend_name) - 1);
            return 1;
        }
    }
    
    // Try X11 - xsel
    if (check_command("xsel")) {
        snprintf(cmd, sizeof(cmd), 
                 "printf '%%s' '%s' | xsel --clipboard --input", escaped);
        if (system(cmd) == 0) {
            strncpy(backend_name, "xsel", sizeof(backend_name) - 1);
            return 1;
        }
    }
    
    return 0;
}

int clipboard_clear(void) {
    return clipboard_copy("");
}

int clipboard_copy_with_timeout(const char *text, int seconds) {
    if (!clipboard_copy(text)) return 0;
    
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        return 0;
    } else if (pid == 0) {
        // Child process
        sleep(seconds);
        clipboard_clear();
        exit(0);
    }
    
    // Parent process continues
    return 1;
}

int clipboard_is_available(void) {
    return check_command("wl-copy") || 
           check_command("pbcopy") || 
           check_command("xclip") || 
           check_command("xsel");
}

#endif

const char* clipboard_get_backend(void) {
    if (strcmp(backend_name, "unknown") == 0) {
        clipboard_is_available(); // Try to detect
    }
    return backend_name;
}
