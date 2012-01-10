#include <ctype.h>
#include <string.h>
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

int is_binary(const void* buf, const int buf_len) {
    int suspicious_bytes = 0;
    int total_bytes = buf_len > 1024 ? 1024 : buf_len;
    const unsigned char *buf_c = buf;
    int i;

    for (i = 0; i < buf_len && i < 1024; i++) {
        if (buf_c[i] == '\0') {
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
