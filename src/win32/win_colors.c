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

// white on black without any attribute
#define COLOR_FALLBACK (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

long win_get_current_screen_color() {
      CONSOLE_SCREEN_BUFFER_INFO sbi;
      HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

      if (console && GetConsoleScreenBufferInfo(console, &sbi))
          return sbi.wAttributes;

      return COLOR_FALLBACK;
}

void win_set_current_screen_color(long c) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console)
        SetConsoleTextAttribute(console, c);
}

static void win_apply_ansi_color_value(int code, long *color, const long def);

// see comments at win_colors.h
// in a nutshell, as long as we see a number, apply it as ansi color code and skip
// to after the next ';', rinse and repeat.
long win_color_from_ansi_values(const char *seq, const long initial, const long def) {
    long color = initial;
    while (seq && isdigit(*seq)) {
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
#define FRGBI (FRGB | FI)

#define BR BACKGROUND_RED
#define BG BACKGROUND_GREEN
#define BB BACKGROUND_BLUE
#define BI BACKGROUND_INTENSITY
#define BRGB (BR | BG | BB)
#define BRGBI (BRGB | BI)

// mask1/2 must be 1 bit
static void swap_bits(long *val, long mask1, long mask2) {
    long t1 = *val & mask1;

    if (*val & mask2)
        *val |=  mask1;
    else
        *val &= ~mask1;

    if (t1)
        *val |=  mask2;
    else
        *val &= ~mask2;
}

// see comment at win_colors.h
long win_attributes_into_color(long color) {
    if(color & CREV) {
        color &= ~CREV;
        swap_bits(&color, FR, BR);
        swap_bits(&color, FG, BG);
        swap_bits(&color, FB, BB);
        swap_bits(&color, FI, BI);
    }

    if (color & CUND) {
        color &= ~CUND;
        color |= FI;
    }

    return color;
}

// native windows console supports 16 colors (bits: r, g, b, brightness) for fg/bg
// and two global bits: inverse and underline (both don't seem to work) - overall 10 bits.
// information gathered by experimenting and from:
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms682088%28v=vs.85%29.aspx#_win32_character_attributes
// - http://unix.stackexchange.com/questions/93814/cant-apply-brightness-to-terminals-background-color
// - https://code.google.com/p/conemu-maximus5/wiki/AnsiEscapeCodes
static void win_apply_ansi_color_value(int code, long *color, const long def) {
    long embedded_intensity = 0;
    if ((code >= 90 && code <= 97) || (code >= 100 && code <= 107)) {
        // fg, bg bright colors - aixterm
        embedded_intensity = (code < 100) ? FI : BI;
        code -= 60;
    }

    switch (code) {
        // default colors with all attributes off
        case  0: *color = def & (FRGB | BRGB); break;

        case  1: *color |=  FI;   break; // fg intensity on
        case  5: *color |=  BI;   break; // blink on -> bg intensity on

        case  2:                         // dim -> fg intensity off
        case 22: *color &= ~FI;   break; // normal intensity -> fg intensity off
        case 25: *color &= ~BI;   break; // blink off -> bg intensity off

        // reverse/underscore are unreliable (completely b0rked?). see apply_attributes
        case  3:                         // italics (/reverse) on
        case  7: *color |=  CREV; break; // reverse on

        case 23:                         // italics (/reverse) off
        case 27: *color &= ~CREV; break; // reverse off

        case  4: *color |=  CUND; break; // underline on
        case 24: *color &= ~CUND; break; // off

        // foreground colors - without affecting fg intensity
        case 30: *color = (*color & ~FRGB)                 ; break;
        case 31: *color = (*color & ~FRGB)  | FR           ; break;
        case 32: *color = (*color & ~FRGB)  |      FG      ; break;
        case 33: *color = (*color & ~FRGB)  | FR | FG      ; break;
        case 34: *color = (*color & ~FRGB)  |           FB ; break;
        case 35: *color = (*color & ~FRGB)  | FR |      FB ; break;
        case 36: *color = (*color & ~FRGB)  |      FG | FB ; break;
        case 37: *color = (*color & ~FRGB)  | FR | FG | FB ; break;

        // reset foreground color and intensity to default
        case 39: *color = (*color & ~FRGBI) | (def & FRGBI); break;

        // background colors - also turn bg intensity off
        case 40: *color = (*color & ~BRGBI)                ; break;
        case 41: *color = (*color & ~BRGBI) | BR           ; break;
        case 42: *color = (*color & ~BRGBI) |      BG      ; break;
        case 43: *color = (*color & ~BRGBI) | BR | BG      ; break;
        case 44: *color = (*color & ~BRGBI) |           BB ; break;
        case 45: *color = (*color & ~BRGBI) | BR |      BB ; break;
        case 46: *color = (*color & ~BRGBI) |      BG | BB ; break;
        case 47: *color = (*color & ~BRGBI) | BR | BG | BB ; break;

        // reset background color and intensity to default
        case 49: *color = (*color & ~BRGBI) | (def & BRGBI); break;
    }

    *color |= embedded_intensity;
}

#endif /* _Win32 */
