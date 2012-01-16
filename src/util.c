#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include "util.h"

/* Blatantly stolen from darwin source code and modified for my own purposes
   TODO: take a look at boyer-moore string searching
 */
char* ag_strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return(NULL);
            } while (sc != c);
            if (len > slen)
                return(NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return((char *)s);
}

char* ag_strncasestr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return(NULL);
            } while (tolower(sc) != tolower(c));
            if (len > slen)
                return(NULL);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return((char *)s);
}

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[]) {
    size_t i = 0;

    for (i = 0; i < 256; i++) {
        skip_lookup[i] = f_len - 1;
    }

    for (i = 0; i < f_len - 1; i++) {
        skip_lookup[(unsigned char)find[i]] = f_len - i - 1;
    }
}

/* Boyer-Moore-Horspool strstr */
char* ag_boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]) {
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

        pos += skip_lookup[(unsigned char)s[pos + i]];
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

/*
 * These functions are taken from Linux. Renamed so there's no
 * possible function name conflicts.
 */
size_t ag_strlcat(char *dest, const char *src, size_t count)
{
    size_t dsize = strlen(dest);
    size_t len = strlen(src);
    size_t res = dsize + len;

    dest += dsize;
    count -= dsize;

    if (len >= count)
        len = count - 1;

    memcpy(dest, src, len);

    dest[len] = 0;

    return res;
}

size_t ag_strlcpy(char *dest, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size)
    {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }

    return ret;
}
