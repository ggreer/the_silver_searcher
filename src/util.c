#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"


void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[], int case_sensitive) {
    size_t i;

    for (i = 0; i < 256; i++) {
        skip_lookup[i] = f_len;
    }

    f_len--;

    for (i = 0; i < f_len; i++) {
        if (case_sensitive) {
            skip_lookup[(unsigned char)find[i]] = f_len - i;
        }
        else {
            skip_lookup[(unsigned char)tolower(find[i])] = f_len - i;
            skip_lookup[(unsigned char)toupper(find[i])] = f_len - i;
        }
    }
}

/* Boyer-Moore-Horspool strstr */
char* boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return NULL;
    }

    while (pos <= (s_len - f_len)) {
        for (i = f_len - 1; s[pos + i] == find[i]; i--) {
            if (i == 0) {
                return (char *)(&(s[pos]));
            }
        }
        pos += skip_lookup[(unsigned char)s[pos + f_len - 1]];
    }

    return NULL;
}

/* Copy-pasted from above. Yes I know this is bad. One day I might even fix it. */
char* boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]) {
    size_t i;
    size_t pos = 0;

    /* It's impossible to match a larger string */
    if (f_len > s_len) {
        return NULL;
    }

    while (pos <= (s_len - f_len)) {
        for (i = f_len - 1; tolower(s[pos + i]) == tolower(find[i]); i--) {
            if (i == 0) {
                return (char *)(&(s[pos]));
            }
        }
        pos += skip_lookup[(unsigned char)s[pos + f_len - 1]];
    }

    return NULL;
}

strncmp_fp get_strstr(cli_options opts) {
    strncmp_fp ag_strncmp_fp = &boyer_moore_strnstr;

    if (opts.casing == CASE_INSENSITIVE) {
        ag_strncmp_fp = &boyer_moore_strncasestr;
    }

    return ag_strncmp_fp;
}

int invert_matches(match matches[], int matches_len, const int buf_len) {
    int i;

    /* this will totally screw-up if a match starts at the very beginning or end of a file */
    matches[matches_len].start = buf_len-1;
    for (i = matches_len; i >= 0; i--) {
        matches[i].end = matches[i].start;
        matches[i].start = i == 0 ? 0 : matches[i-1].end;
    }

    return (matches_len + 1);
}

void compile_study(pcre **re, pcre_extra **re_extra, char *q, const int pcre_opts, const int study_opts) {
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;

    *re = pcre_compile(q, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (re == NULL) {
        log_err("pcre_compile failed at position %i. Error: %s", pcre_err_offset, pcre_err);
        exit(2);
    }
    *re_extra = pcre_study(*re, study_opts, &pcre_err);
    if (re_extra == NULL) {
        log_debug("pcre_study returned nothing useful. Error: %s", pcre_err);
    }
}

/* This function is very hot. It's called on every file. */
int is_binary(const void* buf, const int buf_len) {
    int suspicious_bytes = 0;
    int total_bytes = buf_len > 1024 ? 1024 : buf_len;
    const unsigned char *buf_c = buf;
    int i;

    if (buf_len == 0) {
        return 0;
    }

    for (i = 0; i < total_bytes; i++) {
        if (buf_c[i] == '\0') {
            /* NULL char. It's binary */
            return 1;
        }
        else if ((buf_c[i] < 7 || buf_c[i] > 14) && (buf_c[i] < 32 || buf_c[i] > 127)) {
            suspicious_bytes++;
            /* Disk IO is so slow that it's worthwhile to do this calculation after every suspicious byte. */
            /* This is true even on a 1.6Ghz Atom with an Intel 320 SSD. */
            if ((suspicious_bytes * 100) / total_bytes > 10) {
                return 1;
            }
        }
    }

    return 0;
}

int is_regex(const char* query) {
    char regex_chars[] = {
        '$',
        '(',
        ')',
        '*',
        '+',
        '.',
        '?',
        '[',
        '\\',
        '^',
        '{',
        '|',
        '\0'
    };

    return (strpbrk(query, regex_chars) != NULL);
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

    return (strpbrk(filename, fnmatch_chars) != NULL);
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

int is_whitespace(const char ch) {
    int i;
    char whitespace_chars[] = {
        ' ',
        '\f',
        '\t',
        '\n',
        '\r',
        '\v',
        '\0'
    };

    for (i = 0; whitespace_chars[i] != '\0'; i++) {
        if (ch == whitespace_chars[i]) {
            return TRUE;
        }
    }

    return FALSE;
}

int contains_uppercase(const char* s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (isupper(s[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

#ifndef HAVE_GETLINE
/*
 * Do it yourself getline() implementation
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t len = 0;
    char *srcln = NULL;
    char *newlnptr = NULL;

    /* get line, bail on error */
    if (!(srcln = fgetln(stream, &len))) {
        return -1;
    }

    if (len >= *n) {
        /* line is too big for buffer, must realloc */
        /* double the buffer, bail on error */
        if (!(newlnptr = realloc(*lineptr, len * 2))) {
            return -1;
        }
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
