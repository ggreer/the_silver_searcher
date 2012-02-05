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
        skip_lookup[i] = f_len - 1;
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

void invert_matches(match matches[], int matches_len, const int buf_len) {
    int i;

    if (matches_len == 0) {
        matches[0].start = 0;
        matches[0].end = buf_len;
        matches_len = 1;
        return;
    }

    matches[matches_len].start = buf_len;
    for (i = 0; i < matches_len; i++) {
        matches[i].start = matches[i].end;
        matches[i].end = matches[i+1].start;
    }
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
 * strlcat and strlcpy, taken from linux kernel
 */
size_t strlcat(char *dest, const char *src, size_t count)
{
    size_t dsize = strlen(dest);
    size_t len = strlen(src);
    size_t res = dsize + len;

    dest += dsize;
    count -= dsize;

    if (len >= count) {
        len = count - 1;
    }

    memcpy(dest, src, len);

    dest[len] = 0;

    return(res);
}

size_t strlcpy(char *dest, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size)
    {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }

    return(ret);
}
