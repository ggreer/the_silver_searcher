/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * This file is Copyright 2015 Avi Halachmi
 */
#ifdef _WIN32

#include <stdio.h>
#include <ctype.h>
#include <windows.h>

#include "win_colors.h"


void win_set_ansi_screen_color(const char *str) {
    static int initialized = 0, default_color, current_color;
    if (!initialized) {
        initialized = 1;
        current_color = default_color = win_get_current_screen_color_raw();
    }

    current_color = win_color_from_ansi_values(str, current_color, default_color);
    win_set_current_screen_color_raw(win_attributes_into_color(current_color));
}


// white on black without any attribute
#define COLOR_FALLBACK (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

int win_get_current_screen_color_raw() {
      CONSOLE_SCREEN_BUFFER_INFO sbi;
      HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

      if (console && GetConsoleScreenBufferInfo(console, &sbi))
          return sbi.wAttributes;

      return COLOR_FALLBACK;
}

void win_set_current_screen_color_raw(int c) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console)
        SetConsoleTextAttribute(console, c);
}

static void win_apply_ansi_color_value(int code, int *color, const int def);

// see comments at win_colors.h
// in a nutshell, as long as we see a number, apply it as ansi color code and skip
// to after the next ';'. rinse and repeat.
int win_color_from_ansi_values(const char *seq, const int initial, const int def) {
    int color = initial;
    if (seq && !seq[0])
        seq = "0";  // empty string is an alias for reset

    while (seq && isdigit(seq[0])) {
        int code = atoi(seq);
        win_apply_ansi_color_value(code, &color, def);
        seq = strstr(seq, ";");
        if (seq)
            seq++;
    }
    return color;
}

const char *ansi_first_color_value(const char *ansi_escape) {
    static const char *kAnsiBegin = "\033[";
    char *found = strstr(ansi_escape, kAnsiBegin);
    if (found)
      return found + strlen(kAnsiBegin);
    return 0;
}

// less verbose names to make the code more readable.
#define CREV COMMON_LVB_REVERSE_VIDEO
#define CUND COMMON_LVB_UNDERSCORE

#define FR FOREGROUND_RED
#define FG FOREGROUND_GREEN
#define FB FOREGROUND_BLUE
#define FI FOREGROUND_INTENSITY
#define FRGB (FR | FG | FB)

#define BR BACKGROUND_RED
#define BG BACKGROUND_GREEN
#define BB BACKGROUND_BLUE
#define BI BACKGROUND_INTENSITY
#define BRGB (BR | BG | BB)

// mask1/2 must be 1 bit
static void swap_bits(int *val, int mask1, int mask2) {
    int t1 = *val & mask1;

    if (*val & mask2)
        *val |=  mask1;
    else
        *val &= ~mask1;

    if (t1)
        *val |=  mask2;
    else
        *val &= ~mask2;
}

// squash the underline + inverse bits into the color and clear them.
// see comment at win_colors.h
int win_attributes_into_color(int color) {
    if (color & CUND) {
        color &= ~CUND;
        color |= BI;
    }

    if (color & CREV) {
        color &= ~CREV;
        swap_bits(&color, FR, BR);
        swap_bits(&color, FG, BG);
        swap_bits(&color, FB, BB);
        swap_bits(&color, FI, BI);  // also swap intensity - like gnome-terminal
    }

    return color;
}

// native windows console supports 16 colors (bits: r, g, b, brightness) for fg/bg
// and two global bits: inverse and underline (both don't seem to work) -> 10 bits.
// Since fg intensity is widely used but bg intensity isn't really defined, we map
// rare attributes (blink/underline) to bg intensity, except italics -> inverse.
// supports properly:
// - 8 foreground colors and intensity bit
// - 8 background colors (no intensity since ansi doesn't seem to support it)
// - reset (also clears intensities, inverse and other implied attributes)
// - inverse bit
// - blink -> background intensity bit (same as ansicon and less for windows)
// the code also maps the following which may cross-affect attributes:
// - does NOT treat aixterm colors as atomic -> also turns intensity bit on.
// - dim -> turn off foreground intensity
// when using win_attributes_into_color to interpret inverse/underline bits:
// - underline -> background intensity
// - inverse includes intensities and applied last (so preserves the "uniqueness"
//   of blink/underline independently from fg intensity).
//
// gathered and deduced by experimenting and from:
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms682088%28v=vs.85%29.aspx#_win32_character_attributes
// - http://unix.stackexchange.com/questions/93814/cant-apply-brightness-to-terminals-background-color
// - https://code.google.com/p/conemu-maximus5/wiki/AnsiEscapeCodes
static void win_apply_ansi_color_value(int code, int *color, const int def) {
    int embedded_intensity = 0;
    if ((code >= 90 && code <= 97) || (code >= 100 && code <= 107)) {
        // fg, bg bright colors - aixterm. these should be treated as atomic bright
        // colors such that 0;93;33 should end up as 0;33, but to simplify the code
        // we instead treat 93 as 1;33, so 0;93;33 ends up bright like 0;1;33
        embedded_intensity = (code < 100) ? FI : BI;
        code -= 60;
    }

    switch (code) {
        // default colors with all attributes off
        case  0: *color = def & (FRGB | BRGB); break;

        case  1: *color |=  FI;   break; // fg intensity on

        case  2:                         // dim -> fg intensity off
        case 22: *color &= ~FI;   break; // normal intensity -> fg intensity off

        case  5: *color |=  BI;   break; // blink on -> bg intensity on

        case 25: *color &= ~BI;   break; // blink off -> bg intensity off

        // inverse/underline bits (see win_attributes_into_color)
        case  3:                         // italics (/reverse) on
        case  7: *color |=  CREV; break; // reverse on

        case 23:                         // italics (/reverse) off
        case 27: *color &= ~CREV; break; // reverse off

        case  4: *color |=  CUND; break; // underline on

        case 24: *color &= ~CUND; break; // off

        // the following fg/bg colors/reset don't affect attributes

        // foreground colors
        case 30: *color = (*color & ~FRGB)                ; break;
        case 31: *color = (*color & ~FRGB) | FR           ; break;
        case 32: *color = (*color & ~FRGB) |      FG      ; break;
        case 33: *color = (*color & ~FRGB) | FR | FG      ; break;
        case 34: *color = (*color & ~FRGB) |           FB ; break;
        case 35: *color = (*color & ~FRGB) | FR |      FB ; break;
        case 36: *color = (*color & ~FRGB) |      FG | FB ; break;
        case 37: *color = (*color & ~FRGB) | FR | FG | FB ; break;

        // reset foreground color to default
        case 39: *color = (*color & ~FRGB) | (def & FRGB); break;

        // background colors
        case 40: *color = (*color & ~BRGB)                ; break;
        case 41: *color = (*color & ~BRGB) | BR           ; break;
        case 42: *color = (*color & ~BRGB) |      BG      ; break;
        case 43: *color = (*color & ~BRGB) | BR | BG      ; break;
        case 44: *color = (*color & ~BRGB) |           BB ; break;
        case 45: *color = (*color & ~BRGB) | BR |      BB ; break;
        case 46: *color = (*color & ~BRGB) |      BG | BB ; break;
        case 47: *color = (*color & ~BRGB) | BR | BG | BB ; break;

        // reset background color to default
        case 49: *color = (*color & ~BRGB) | (def & BRGB); break;
    }

    *color |= embedded_intensity;
}

#endif /* _Win32 */
