#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>
#include <stdarg.h>
#include <strings.h>
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
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
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
                    return (NULL);
            } while (tolower(sc) != tolower(c));
            if (len > slen)
                return (NULL);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

/* Boyer-Moore-Horspool strstr */
char* ag_boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len) {
    int i;
    size_t skip_lookup[256];
    char *haystack = s;

    if (f_len > s_len) {
        return(NULL);
    }

    for (i = 0; i < 256; i++) {
        skip_lookup[i] = f_len - 1;
    }

    /* TODO: move the lookup generation outside this function */
    for (i = f_len - 1; i >= 0; i--) {
        skip_lookup[find[i]] = f_len - i;
        printf("skip_lookup[%c] = %i\n", find[i], (int)(f_len - i));
    }

    while (s_len > f_len) {
        for (i = 0; haystack[i] == find[i]; i++) {
            printf("haystack %c find %c", haystack[i], find[i]);
            if (i == (int)f_len - 1) {
                printf("match found at position %i, returning\n", i);
                return haystack;
            }
        }

        haystack += skip_lookup[(int)*haystack];
        s_len -= skip_lookup[(int)*s];
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
        else if (buf_c[i] < 32 || buf_c[i] > 128) {
            suspicious_bytes++;
        }
    }

    /* If > 10% of bytes are suspicious, assume it's binary */
    if ((suspicious_bytes * 100) / total_bytes > 10) {
        return(1);
    }

    return(0);
}
