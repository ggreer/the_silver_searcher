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
char* boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return(NULL);
    }

    while (pos < (s_len - f_len)) {
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
char* boyer_moore_strncasestr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return(NULL);
    }

    while (pos < (s_len - f_len)) {
        for (i = f_len - 1; tolower(s[pos + i]) == tolower(find[i]); i--) {
            if (i == 0) {
                return((char *)(&(s[pos])));
            }
        }

        pos += skip_lookup[(unsigned char)tolower(s[pos + f_len - 1])];
    }

    return(NULL);
}

int is_binary(const void* buf, const int buf_len) {
    int suspicious_bytes = 0;
    int total_bytes = buf_len > 1024 ? 1024 : buf_len;
    const unsigned char *buf_c = buf;
    int i;

    for (i = 0; i < buf_len && i < 1024; i++) {
        if (buf_c[i] == '\0') {
            /* NULL char. It's binary */
            return(1);
        }
        else if (buf_c[i] < 32 || buf_c[i] > 127) {
            suspicious_bytes++;
        }
    }

    /* If > 10% of bytes are suspicious, assume it's binary */
    if ((suspicious_bytes * 100) / total_bytes > 10) {
        return(1);
    }

    return(0);
}

int is_regex(const char* query, const int query_len) {
    int i, j;
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

    for (i = 0; i < query_len; i++) {
        for (j = 0; regex_chars[j] != '\0'; j++) {
            if (query[i] == regex_chars[j]) {
                return(1);
            }
        }
    }

    return(0);
}

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
        *d = '\0';      /* NUL-terminate dst */

      while (*s++)
        ;
    }

    return (s - src - 1);   /* count does not include NUL */
}

/*
 * Do it yourself getline() implementation
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    ssize_t len = 0;
    size_t last = 0;

    do {
        *n += BUFSIZ; /* "the optimal read size for this platform" */
        lineptr = realloc(*lineptr, *n);
        fgets(*lineptr + last, *n, stream);
        len = strlen(*lineptr);
        last = len - 1;
    } while (!feof(stream) && *lineptr[last] != '\n'); /* this will break on Windows */
    return len;
}
