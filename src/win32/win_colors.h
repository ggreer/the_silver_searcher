#ifdef _WIN32

#ifndef WIN_COLORS_H
#define WIN_COLORS_H

long win_get_current_screen_color(void);
void win_set_current_screen_color(long c);

// seq is only the numbers separated by semicolons, e.g. "1;33" should be used as \033[1;33m
//   there's no windows API to set/get defaults (`def`), so it has to be provided.
// the `initial` color CAN be read from the screen buffer, but we leave it to the
//   caller to do that or use another (possibly the last which the caller used).
// note that we don't try to detect xterm256 or 24b codes (38;5;N 48;5;N 38;2;R;G;B 48;2;R;G;B)
//   and therefore we'll produce incorrect colors on such cases. It's technically possible to
//   handle it gracefully (ignore or quantize the colors to 4 bits), but for now we don't.
// returns the color which we ended up with after using the sequence - without applying it.
long win_color_from_ansi_values(const char *seq, const long initial, const long def);

// returns a pointer to the first value after the first "\033[", or null.
// it's not a replacement for proper ansi escaping, but it's enough to
// use the first color escape sequence in a string.
const char *ansi_first_color_value(const char *ansi_escape);

#endif

#endif
