#ifndef CLIPBOARD_H
#define CLIPBOARD_H

/**
 * Cross-platform clipboard management
 * Supports: Linux (X11/Wayland), macOS, Windows
 */

// Copy text to clipboard
// Returns: 1 on success, 0 on failure
int clipboard_copy(const char *text);

// Clear clipboard contents
// Returns: 1 on success, 0 on failure
int clipboard_clear(void);

// Copy text to clipboard and automatically clear after timeout
// Parameters:
//   text: Text to copy
//   seconds: Time in seconds before auto-clear (recommended: 30-45)
// Returns: 1 on success, 0 on failure
int clipboard_copy_with_timeout(const char *text, int seconds);

// Check if clipboard functionality is available
// Returns: 1 if available, 0 if not
int clipboard_is_available(void);

// Get clipboard backend name (for debugging/info)
// Returns: String with backend name ("xclip", "pbcopy", "native", etc)
const char* clipboard_get_backend(void);

#endif // CLIPBOARD_H
