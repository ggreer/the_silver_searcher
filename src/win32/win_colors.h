#ifdef _WIN32

#ifndef WIN_COLORS_H
#define WIN_COLORS_H

// colors are represented by WORD (16 bit on windows) and int is signed 32b

// ------------- High level API --------------

// This encapsulates initialization (when called for the first time) and tracking
// of the last used color. May behave somewhat unexpectedly if the screen colors
// were modified via other means between calls, though "0" (reset) will still
// work, as long as it was first called when the screen was at its default color.
// - accepts only plain number strings such as "0;33;42"
// - empty string or null handled gracefully
// - not thread safe - may lose tracking of last color if not synchronized.
void win_set_ansi_screen_color(const char *str);

// Naive parsing utility: returns a pointer to the first char after the
// first "\033[", or null. it's not a replacement for proper ansi unescaping,
// but it should be enough if the string is known to be a color escape sequence.
const char *ansi_first_color_value(const char *ansi_escape);


// ------------- Low level API ---------------

// It's recommended to read the screen color only once - to use it as default
// color on all subsequent color calls, and and from then on use the last
// generated color as an initial value for the next color call.
// that's because several attributes are mapped into only two bits (fg/bg brightness)
// and so reading the color back from the screen will only "see" fg/bg color and
// intensity, but will not be able to map back to inverse/underline/blink and so
// the next color command may manipulate attributes incorrectly.


// with whatever attributes it happens to have
int win_get_current_screen_color_raw(void);

// Together with the (typically/always not working) inverse/underline bits
void win_set_current_screen_color_raw(int c);

// seq is only the numbers separated by semicolons, e.g. "1;33".
//   there's no windows API to set/get defaults (`def`), so it has to be provided.
// the `initial` color CAN be read from the screen buffer, but we leave it to the
//   caller to do so or use another (possibly the last which the caller used,
//   see also win_attributes_into_color).
// note that we don't try to detect xterm256 or 24b codes (38;5;N 48;5;N 38;2;R;G;B 48;2;R;G;B)
//   and therefore we'll produce incorrect colors on such cases. It's technically possible to
//   handle it gracefully (ignore or quantize the colors to 4 bits), but for now we don't.
// returns the color which we ended up with after using the sequence - without applying it.
int win_color_from_ansi_values(const char *seq, const int initial, const int def);

// win_color_from_ansi_values includes reverse/underline bits which are typically
//   ignored when used, so provide a method to apply them into the actual colors.
// Applies the underline bit as background intensity and clears the bit.
// Applies the inverse bit by swapping fg/bg color + intensity and clears the bit.
// Note: win_get_current_screen_color will NOT be able to read these attributes
//       from screen because they've been stripped and applied directly to the color.
//       to keep the attributes state, use the original color (which includes these
//       bits) as 'initial' at the next call to win_color_from_ansi_values(...).
//       (however, color '0' with the original 'default' doesn't depend on state)
int win_attributes_into_color(int color);


#endif

#endif
