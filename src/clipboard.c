#include "clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <errno.h>
#endif

static char backend_name[32] = "unknown";
static int backend_detected = 0;

#ifdef _WIN32
// Windows implementation using native API (unchanged - already safe)
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
    backend_detected = 1;
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
    if (!backend_detected) {
        strncpy(backend_name, "native", sizeof(backend_name) - 1);
        backend_detected = 1;
    }
    return 1; // Windows always has clipboard
}

#else
// Unix/Linux/macOS implementation using SAFE pipe/fork/exec pattern

// Check if command exists in PATH
static int check_command(const char *command) {
    char path_env[4096];
    const char *path = getenv("PATH");
    if (!path) return 0;
    
    strncpy(path_env, path, sizeof(path_env) - 1);
    path_env[sizeof(path_env) - 1] = '\0';
    
    char *dir = strtok(path_env, ":");
    while (dir) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        
        if (access(full_path, X_OK) == 0) {
            return 1;
        }
        
        dir = strtok(NULL, ":");
    }
    
    return 0;
}

// SECURE clipboard copy using pipe/fork/exec (NO SHELL)
static int safe_clipboard_copy(const char *text, const char *command, 
                               const char *backend) {
    if (!text || strlen(text) == 0) return 0;
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[1]); // Close write end
        
        // Redirect stdin to read from pipe
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        
        close(pipefd[0]);
        
        // Execute command WITHOUT shell - prevents injection
        if (strcmp(command, "wl-copy") == 0) {
            execlp("wl-copy", "wl-copy", NULL);
        } else if (strcmp(command, "pbcopy") == 0) {
            execlp("pbcopy", "pbcopy", NULL);
        } else if (strcmp(command, "xclip") == 0) {
            execlp("xclip", "xclip", "-selection", "clipboard", NULL);
        } else if (strcmp(command, "xsel") == 0) {
            execlp("xsel", "xsel", "--clipboard", "--input", NULL);
        }
        
        // If exec fails
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    
    // Parent process
    close(pipefd[0]); // Close read end
    
    // Write text to clipboard command via pipe
    size_t text_len = strlen(text);
    ssize_t written = write(pipefd[1], text, text_len);
    
    close(pipefd[1]);
    
    if (written != (ssize_t)text_len) {
        perror("write");
        waitpid(pid, NULL, 0); // Reap child
        return 0;
    }
    
    // Wait for child to complete
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 0;
    }
    
    // Check if command succeeded
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        strncpy(backend_name, backend, sizeof(backend_name) - 1);
        backend_name[sizeof(backend_name) - 1] = '\0';
        backend_detected = 1;
        return 1;
    }
    
    return 0;
}

int clipboard_copy(const char *text) {
    if (!text || strlen(text) == 0) return 0;
    
    // Try Wayland first (wl-copy)
    if (check_command("wl-copy")) {
        if (safe_clipboard_copy(text, "wl-copy", "wl-copy")) {
            return 1;
        }
    }
    
    // Try macOS (pbcopy)
    if (check_command("pbcopy")) {
        if (safe_clipboard_copy(text, "pbcopy", "pbcopy")) {
            return 1;
        }
    }
    
    // Try X11 - xclip
    if (check_command("xclip")) {
        if (safe_clipboard_copy(text, "xclip", "xclip")) {
            return 1;
        }
    }
    
    // Try X11 - xsel
    if (check_command("xsel")) {
        if (safe_clipboard_copy(text, "xsel", "xsel")) {
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
    // Detect backend if not already detected
    if (!backend_detected) {
        if (check_command("wl-copy")) {
            strncpy(backend_name, "wl-copy", sizeof(backend_name) - 1);
            backend_detected = 1;
            return 1;
        }
        
        if (check_command("pbcopy")) {
            strncpy(backend_name, "pbcopy", sizeof(backend_name) - 1);
            backend_detected = 1;
            return 1;
        }
        
        if (check_command("xclip")) {
            strncpy(backend_name, "xclip", sizeof(backend_name) - 1);
            backend_detected = 1;
            return 1;
        }
        
        if (check_command("xsel")) {
            strncpy(backend_name, "xsel", sizeof(backend_name) - 1);
            backend_detected = 1;
            return 1;
        }
        
        // No backend found
        strncpy(backend_name, "not available", sizeof(backend_name) - 1);
        backend_detected = 1;
        return 0;
    }
    
    return strcmp(backend_name, "not available") != 0;
}

#endif

const char* clipboard_get_backend(void) {
    if (!backend_detected) {
        clipboard_is_available(); // Force detection
    }
    return backend_name;
}
