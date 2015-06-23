#ifndef _OSDEP_H
#define _OSDEP_H

int getnumcores_ag(void);
int fnmatch_path_ag(const char *pattern, const char *string);

#define AG_UNUSED(x) (void)(x)
#define AG_UNUSED2(x,y) (void)(x); (void)(y)

#ifdef _WIN32
#include <stdio.h>

#define flockfile(x)
#define funlockfile(x)
#define getc_unlocked getc

void windows_use_ansi(int use_ansi);

int fprintf_w32(FILE *fp, const char *format, ...);
#define fprintf fprintf_w32

int getpagesize_w32(void);
#define getpagesize getpagesize_w32

#endif /* _WIN32 */
#endif /* _OSDEP_H */
