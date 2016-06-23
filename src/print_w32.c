#ifdef _WIN32

#include "print.h"
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#ifndef FOREGROUND_MASK
#define FOREGROUND_MASK (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#endif
#ifndef BACKGROUND_MASK
#define BACKGROUND_MASK (BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#endif

#define FG_RGB (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define BG_RGB (BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN)

// BUFSIZ is guarenteed to be "at least 256 bytes" which might
// not be enough for us. Use an arbitrary but reasonably big
// buffer. win32 colored output will be truncated to this length.
#define BUF_SIZE (16 * 1024)

// max consecutive ansi sequence values beyond which we're aborting
// e.g. this is 3 values: \e[0;1;33m
#define MAX_VALUES 8

static int g_use_ansi = 0;
void windows_use_ansi(int use_ansi) {
    g_use_ansi = use_ansi;
}

int fprintf_w32(FILE *fp, const char *format, ...) {
    va_list args;
    char buf[BUF_SIZE] = { 0 }, *ptr = buf;
    static WORD attr_reset;
    static BOOL attr_initialized = FALSE;
    HANDLE stdo = INVALID_HANDLE_VALUE;
    WORD attr;
    DWORD written, csize;
    CONSOLE_CURSOR_INFO cci;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD coord;

    // if we don't output to screen (tty) e.g. for pager/pipe or
    // if for other reason we can't access the screen info, of if
    // the user just prefers ansi, do plain passthrough.
    BOOL passthrough =
        g_use_ansi ||
        !isatty(fileno(fp)) ||
        INVALID_HANDLE_VALUE == (stdo = (HANDLE)_get_osfhandle(fileno(fp))) ||
        !GetConsoleScreenBufferInfo(stdo, &csbi);

    if (passthrough) {
        int rv;
        va_start(args, format);
        rv = vfprintf(fp, format, args);
        va_end(args);
        return rv;
    }

    va_start(args, format);
    // truncates to (null terminated) BUF_SIZE if too long.
    // if too long - vsnprintf will fill count chars without
    // terminating null. buf is zeroed, so make sure we don't fill it.
    vsnprintf(buf, BUF_SIZE - 1, format, args);
    va_end(args);

    attr = csbi.wAttributes;
    if (!attr_initialized) {
        // reset is defined to have all (non color) attributes off
        attr_reset = attr & (FG_RGB | BG_RGB);
        attr_initialized = TRUE;
    }

    while (*ptr) {
        if (*ptr == '\033') {
            unsigned char c;
            int i, n = 0, m = '\0', v[MAX_VALUES], w, h;
            for (i = 0; i < MAX_VALUES; i++)
                v[i] = -1;
            ptr++;
        retry:
            if ((c = *ptr++) == 0)
                break;
            if (isdigit(c)) {
                if (v[n] == -1)
                    v[n] = c - '0';
                else
                    v[n] = v[n] * 10 + c - '0';
                goto retry;
            }
            if (c == '[') {
                goto retry;
            }
            if (c == ';') {
                if (++n == MAX_VALUES)
                    break;
                goto retry;
            }
            if (c == '>' || c == '?') {
                m = c;
                goto retry;
            }

            switch (c) {
                // n is the last occupied index, so we have n+1 values
                case 'h':
                    if (m == '?') {
                        for (i = 0; i <= n; i++) {
                            switch (v[i]) {
                                case 3:
                                    GetConsoleScreenBufferInfo(stdo, &csbi);
                                    w = csbi.dwSize.X;
                                    h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
                                    csize = w * (h + 1);
                                    coord.X = 0;
                                    coord.Y = csbi.srWindow.Top;
                                    FillConsoleOutputCharacter(stdo, ' ', csize, coord, &written);
                                    FillConsoleOutputAttribute(stdo, csbi.wAttributes, csize, coord, &written);
                                    SetConsoleCursorPosition(stdo, csbi.dwCursorPosition);
                                    csbi.dwSize.X = 132;
                                    SetConsoleScreenBufferSize(stdo, csbi.dwSize);
                                    csbi.srWindow.Right = csbi.srWindow.Left + 131;
                                    SetConsoleWindowInfo(stdo, TRUE, &csbi.srWindow);
                                    break;
                                case 5:
                                    attr =
                                        ((attr & FOREGROUND_MASK) << 4) |
                                        ((attr & BACKGROUND_MASK) >> 4);
                                    SetConsoleTextAttribute(stdo, attr);
                                    break;
                                case 9:
                                    break;
                                case 25:
                                    GetConsoleCursorInfo(stdo, &cci);
                                    cci.bVisible = TRUE;
                                    SetConsoleCursorInfo(stdo, &cci);
                                    break;
                                case 47:
                                    coord.X = 0;
                                    coord.Y = 0;
                                    SetConsoleCursorPosition(stdo, coord);
                                    break;
                                default:
                                    break;
                            }
                        }
                    } else if (m == '>' && v[0] == 5) {
                        GetConsoleCursorInfo(stdo, &cci);
                        cci.bVisible = FALSE;
                        SetConsoleCursorInfo(stdo, &cci);
                    }
                    break;
                case 'l':
                    if (m == '?') {
                        for (i = 0; i <= n; i++) {
                            switch (v[i]) {
                                case 3:
                                    GetConsoleScreenBufferInfo(stdo, &csbi);
                                    w = csbi.dwSize.X;
                                    h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
                                    csize = w * (h + 1);
                                    coord.X = 0;
                                    coord.Y = csbi.srWindow.Top;
                                    FillConsoleOutputCharacter(stdo, ' ', csize, coord, &written);
                                    FillConsoleOutputAttribute(stdo, csbi.wAttributes, csize, coord, &written);
                                    SetConsoleCursorPosition(stdo, csbi.dwCursorPosition);
                                    csbi.srWindow.Right = csbi.srWindow.Left + 79;
                                    SetConsoleWindowInfo(stdo, TRUE, &csbi.srWindow);
                                    csbi.dwSize.X = 80;
                                    SetConsoleScreenBufferSize(stdo, csbi.dwSize);
                                    break;
                                case 5:
                                    attr =
                                        ((attr & FOREGROUND_MASK) << 4) |
                                        ((attr & BACKGROUND_MASK) >> 4);
                                    SetConsoleTextAttribute(stdo, attr);
                                    break;
                                case 25:
                                    GetConsoleCursorInfo(stdo, &cci);
                                    cci.bVisible = FALSE;
                                    SetConsoleCursorInfo(stdo, &cci);
                                    break;
                                default:
                                    break;
                            }
                        }
                    } else if (m == '>' && v[0] == 5) {
                        GetConsoleCursorInfo(stdo, &cci);
                        cci.bVisible = TRUE;
                        SetConsoleCursorInfo(stdo, &cci);
                    }
                    break;
                case 'm':
                    for (i = 0; i <= n; i++) {
                        if (v[i] == -1 || v[i] == 0)
                            attr = attr_reset;
                        else if (v[i] == 1)
                            attr |= FOREGROUND_INTENSITY;
                        else if (v[i] == 4)
                            attr |= FOREGROUND_INTENSITY;
                        else if (v[i] == 5) // blink is typically applied as bg intensity
                            attr |= BACKGROUND_INTENSITY;
                        else if (v[i] == 7)
                            attr =
                                ((attr & FOREGROUND_MASK) << 4) |
                                ((attr & BACKGROUND_MASK) >> 4);
                        else if (v[i] == 10)
                            ; // symbol on
                        else if (v[i] == 11)
                            ; // symbol off
                        else if (v[i] == 22)
                            attr &= ~FOREGROUND_INTENSITY;
                        else if (v[i] == 24)
                            attr &= ~FOREGROUND_INTENSITY;
                        else if (v[i] == 25)
                            attr &= ~BACKGROUND_INTENSITY;
                        else if (v[i] == 27)
                            attr =
                                ((attr & FOREGROUND_MASK) << 4) |
                                ((attr & BACKGROUND_MASK) >> 4);
                        else if (v[i] >= 30 && v[i] <= 37) {
                            attr &= ~FG_RGB; // doesn't affect attributes
                            if ((v[i] - 30) & 1)
                                attr |= FOREGROUND_RED;
                            if ((v[i] - 30) & 2)
                                attr |= FOREGROUND_GREEN;
                            if ((v[i] - 30) & 4)
                                attr |= FOREGROUND_BLUE;
                        } else if (v[i] == 39) // reset fg color and attributes
                            attr = (attr & ~FOREGROUND_MASK) | (attr_reset & FG_RGB);
                        else if (v[i] >= 40 && v[i] <= 47) {
                            attr &= ~BG_RGB;
                            if ((v[i] - 40) & 1)
                                attr |= BACKGROUND_RED;
                            if ((v[i] - 40) & 2)
                                attr |= BACKGROUND_GREEN;
                            if ((v[i] - 40) & 4)
                                attr |= BACKGROUND_BLUE;
                        } else if (v[i] == 49) // reset bg color
                            attr = (attr & ~BACKGROUND_MASK) | (attr_reset & BG_RGB);
                    }
                    SetConsoleTextAttribute(stdo, attr);
                    break;
                case 'K':
                    GetConsoleScreenBufferInfo(stdo, &csbi);
                    coord = csbi.dwCursorPosition;
                    switch (v[0]) {
                        default:
                        case 0:
                            csize = csbi.dwSize.X - coord.X;
                            break;
                        case 1:
                            csize = coord.X;
                            coord.X = 0;
                            break;
                        case 2:
                            csize = csbi.dwSize.X;
                            coord.X = 0;
                            break;
                    }
                    FillConsoleOutputCharacter(stdo, ' ', csize, coord, &written);
                    FillConsoleOutputAttribute(stdo, csbi.wAttributes, csize, coord, &written);
                    SetConsoleCursorPosition(stdo, csbi.dwCursorPosition);
                    break;
                case 'J':
                    GetConsoleScreenBufferInfo(stdo, &csbi);
                    w = csbi.dwSize.X;
                    h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
                    coord = csbi.dwCursorPosition;
                    switch (v[0]) {
                        default:
                        case 0:
                            csize = w * (h - coord.Y) - coord.X;
                            coord.X = 0;
                            break;
                        case 1:
                            csize = w * coord.Y + coord.X;
                            coord.X = 0;
                            coord.Y = csbi.srWindow.Top;
                            break;
                        case 2:
                            csize = w * (h + 1);
                            coord.X = 0;
                            coord.Y = csbi.srWindow.Top;
                            break;
                    }
                    FillConsoleOutputCharacter(stdo, ' ', csize, coord, &written);
                    FillConsoleOutputAttribute(stdo, csbi.wAttributes, csize, coord, &written);
                    SetConsoleCursorPosition(stdo, csbi.dwCursorPosition);
                    break;
                case 'H':
                    GetConsoleScreenBufferInfo(stdo, &csbi);
                    coord = csbi.dwCursorPosition;
                    if (v[0] != -1) {
                        if (v[1] != -1) {
                            coord.Y = csbi.srWindow.Top + v[0] - 1;
                            coord.X = v[1] - 1;
                        } else
                            coord.X = v[0] - 1;
                    } else {
                        coord.X = 0;
                        coord.Y = csbi.srWindow.Top;
                    }
                    if (coord.X < csbi.srWindow.Left)
                        coord.X = csbi.srWindow.Left;
                    else if (coord.X > csbi.srWindow.Right)
                        coord.X = csbi.srWindow.Right;
                    if (coord.Y < csbi.srWindow.Top)
                        coord.Y = csbi.srWindow.Top;
                    else if (coord.Y > csbi.srWindow.Bottom)
                        coord.Y = csbi.srWindow.Bottom;
                    SetConsoleCursorPosition(stdo, coord);
                    break;
                default:
                    break;
            }
        } else {
            putchar(*ptr);
            ptr++;
        }
    }
    return ptr - buf;
}

#endif /* _WIN32 */
