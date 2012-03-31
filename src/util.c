#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include "util.h"


void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[], int case_sensitive) {
    size_t i = 0;

    for (i = 0; i < 256; i++) {
        skip_lookup[i] = f_len - 1 || 1; /* Handle the case of f_len == 1 */
    }

    for (i = 0; i < f_len - 1; i++) {
        if (case_sensitive) {
            skip_lookup[(unsigned char)find[i]] = f_len - i - 1;
        }
        else {
            skip_lookup[(unsigned char)tolower(find[i])] = f_len - i - 1;
        }
    }
}

/* Boyer-Moore-Horspool strstr */
char* boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return(NULL);
    }

    while (pos <= (s_len - f_len)) {
        for (i = f_len - 1; s[pos + i] == find[i]; i--) {
            if (i == 0) {
                return((char *)(&(s[pos])));
            }
        }
        pos += skip_lookup[(unsigned char)s[pos + f_len - 1]];
    }

    return(NULL);
}

/* Copy-pasted from above. Yes I know this is bad. One day I might even fix it. */
char* boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return(NULL);
    }

    while (pos <= (s_len - f_len)) {
        for (i = f_len - 1; tolower(s[pos + i]) == tolower(find[i]); i--) {
            if (i == 0) {
                return((char *)(&(s[pos])));
            }
        }
        pos += skip_lookup[(unsigned char)tolower(s[pos + f_len - 1])];
    }

    return(NULL);
}

int invert_matches(match matches[], int matches_len, const int buf_len) {
    int i;

    /* this will totally screw-up if a match starts at the very beginning or end of a file */
    matches[matches_len].start = buf_len-1;
    for (i = matches_len; i >= 0; i--) {
        matches[i].end = matches[i].start;
        matches[i].start = i == 0 ? 0 : matches[i-1].end;
    }

    return(matches_len + 1);
}

/* This function is very hot. It's called on every file. */
int is_binary(const void* buf, const int buf_len) {
    int suspicious_bytes = 0;
    int total_bytes = buf_len > 1024 ? 1024 : buf_len;
    const unsigned char *buf_c = buf;
    int i;

    if (buf_len == 0) {
        return(0);
    }

    for (i = 0; i < buf_len && i < 1024; i++) {
        if (buf_c[i] == '\0') {
            /* NULL char. It's binary */
            return(1);
        }
        else if ((buf_c[i] < 7 || buf_c[i] > 14) && (buf_c[i] < 32 || buf_c[i] > 127)) {
            suspicious_bytes++;
            /* disk IO is so slow that it's worthwhile to do this calculation after every suspicious byte */
            /* even on a 1.6Ghz Atom with an intel 320 SSD */
            if ((suspicious_bytes * 100) / total_bytes > 10) {
                return(1);
            }
        }
    }

    return(0);
}

int has_chars(const char* s, const char* chars) {
    int i, j;
    for (i = 0; s[i] != '\0'; i++) {
        for (j = 0; chars[j] != '\0'; j++) {
            if (s[i] == chars[j]) {
                return(1);
            }
        }
    }

    return(0);
}

int is_regex(const char* query) {
    char regex_chars[] = {
        '\\',
        '^',
        '$',
        '.',
        '[',
        '|',
        '(',
        ')',
        '?',
        '*',
        '+',
        '{',
        '\0'
    };

    return(has_chars(query, regex_chars));
}

int is_fnmatch(const char* filename) {
    char fnmatch_chars[] = {
        '!',
        '*',
        '?',
        '[',
        ']',
        '\0'
    };

    return(has_chars(filename, fnmatch_chars));
}

int binary_search(const char* needle, char **haystack, int start, int end) {
    int mid;
    int rc;

    if (start == end) {
        return -1;
    }

    mid = (start + end) / 2; /* can screw up on arrays with > 2 billion elements */

    rc = strcmp(needle, haystack[mid]);
    if (rc < 0) {
        return binary_search(needle, haystack, start, mid);
    }
    else if (rc > 0) {
        return binary_search(needle, haystack, mid + 1, end);
    }

    return mid;
}

#ifndef HAVE_STRLCAT
/*
 * strlcat and strlcpy, taken from Apache Traffic Server
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;
    size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
      d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
      return (dlen + strlen(s));
    while (*s != '\0') {
      if (n != 1) {
        *d++ = *s;
        n--;
      }
      s++;
    }
    *d = '\0';

    return (dlen + (s - src));  /* count does not include NUL */
}
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
      while (--n != 0) {
        if ((*d++ = *s++) == '\0')
          break;
      }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
      if (siz != 0)
        *d = '\0'; /* NUL-terminate dst */

      while (*s++)
        ;
    }

    return (s - src - 1);   /* count does not include NUL */
}
#endif

#ifndef HAVE_GETLINE
/*
 * Do it yourself getline() implementation
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t len = 0;
    char *srcln = NULL;
    char *newlnptr = NULL;

    if (!(srcln = fgetln(stream, &len))) /* get line, bail on error */
        return -1;

    if (len >= *n) { /* line is too big for buffer, must realloc */
        if(!(newlnptr = realloc(*lineptr, len * 2))) /* double the buffer, bail on error */
          return -1;
        *lineptr = newlnptr; 
        *n = len * 2 ;
    }
    memcpy(*lineptr, srcln, len);
    return len;
}
#endif

#ifndef HAVE_STRNDUP
/* Apache-licensed implementation of strndup for OS
 * taken from http://source-android.frandroid.com/dalvik/tools/dmtracedump/CreateTestTrace.c
 */ 
char *strndup(const char *src, size_t len)
{
    char *dest = (char *) malloc(len + 1);
    strncpy(dest, src, len);
    dest[len] = 0;
    return dest;
}
#endif
