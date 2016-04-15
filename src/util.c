#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "util.h"
#include "config.h"

#ifdef _WIN32
#include <windows.h>
#define flockfile(x)
#define funlockfile(x)
#define getc_unlocked(x) getc(x)
#endif

#define CHECK_AND_RETURN(ptr)             \
    if (ptr == NULL) {                    \
        die("Memory allocation failed."); \
    }                                     \
    return ptr;

static ag_ds ag_dsmakeroom(ag_ds s, size_t addlen) {
    struct ag_dshdr *sh, *newsh;
    size_t free = ag_dsavail(s);
    size_t len, newlen;

    if (free >= addlen) {
        return s;
    }
    len = ag_dslen(s);
    newlen = len + addlen;
    if (newlen < MAX_AGDS_PREALLOC) {
        newlen *= 2;
    } else {
        newlen += MAX_AGDS_PREALLOC;
    }
    sh = (void *) (s - sizeof(struct ag_dshdr));
    newsh = ag_realloc(sh, sizeof(struct ag_dshdr) + newlen + 1);
    newsh->free = newlen - len;
    return (char *) newsh->buf;
}

ag_ds ag_dsnew(size_t size) {
    struct ag_dshdr *sh;

    sh = ag_calloc(sizeof(struct ag_dshdr) + size + 1, sizeof(char));
    sh->len = 0;
    sh->free = size;
    return (char *)sh->buf;
}

void ag_dsfree(ag_ds s) {
    if (s == NULL) {
        return;
    }
    free(s - sizeof(struct ag_dshdr));
}

void ag_dsreset(ag_ds s) {
    if (s == NULL) {
        return;
    }
    struct ag_dshdr *sh;
    sh = (void *) (s - sizeof(struct ag_dshdr));
    sh->free += sh->len;
    sh->len = 0;
}

int ag_vsprintf(ag_ds *s, const char *format, va_list ap) {
    unsigned int ret;
    struct ag_dshdr *sh = (void *) (*s - sizeof(struct ag_dshdr));
    size_t curlen = ag_dslen(*s);

    /* ret is the real length of the string */
    ret = vsnprintf(*s + sh->len, sh->free + 1, format, ap);
    if (ret >= sh->free + 1) {
        *s = ag_dsmakeroom(*s, ret);
        sh = (void *) (*s - sizeof(struct ag_dshdr));
        ret = vsnprintf(*s + sh->len, sh->free + 1, format, ap);
    }

    sh->len = curlen + ret;
    sh->free -= ret;
    return ret;
}

int ag_dsncat(ag_ds *s, const char *t, size_t len) {
    struct ag_dshdr *sh;
    size_t curlen = ag_dslen(*s);

    *s = ag_dsmakeroom(*s, len);
    sh = (void *) (*s - sizeof(struct ag_dshdr));
    memcpy(*s + sh->len, t, len);
    sh->len = curlen + len;
    sh->free -= len;
    *s[sh->len] = '\0';
    return len;
}

void ag_setspecific(void) {
    ag_specific_t *as = ag_malloc(sizeof(as));
    as->lock = FALSE;
    as->ds = ag_dsnew(INIT_AGDSLEN);
    pthread_setspecific(worker_key, as);
}

void *ag_getspecific(void) {
    return pthread_getspecific(worker_key);
}

void ag_specific_free(void *data) {
    ag_dsfree(((ag_specific_t *)data)->ds);
    free(data);
}

void *ag_malloc(size_t size) {
    void *ptr = malloc(size);
    CHECK_AND_RETURN(ptr)
}

void *ag_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    CHECK_AND_RETURN(new_ptr)
}

void *ag_calloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    CHECK_AND_RETURN(ptr)
}

char *ag_strdup(const char *s) {
    char *str = strdup(s);
    CHECK_AND_RETURN(str)
}

char *ag_strndup(const char *s, size_t size) {
    char *str = NULL;
#ifdef HAVE_STRNDUP
    str = strndup(s, size);
    CHECK_AND_RETURN(str)
#else
    str = (char *)ag_malloc(size + 1);
    strlcpy(str, s, size + 1);
    return str;
#endif
}

void free_strings(char **strs, const size_t strs_len) {
    if (strs == NULL) {
        return;
    }
    size_t i;
    for (i = 0; i < strs_len; i++) {
        free(strs[i]);
    }
    free(strs);
}

void generate_alpha_skip(const char *find, size_t f_len, size_t skip_lookup[], const int case_sensitive) {
    size_t i;

    for (i = 0; i < 256; i++) {
        skip_lookup[i] = f_len;
    }

    f_len--;

    for (i = 0; i < f_len; i++) {
        if (case_sensitive) {
            skip_lookup[(unsigned char)find[i]] = f_len - i;
        } else {
            skip_lookup[(unsigned char)tolower(find[i])] = f_len - i;
            skip_lookup[(unsigned char)toupper(find[i])] = f_len - i;
        }
    }
}

int is_prefix(const char *s, const size_t s_len, const size_t pos, const int case_sensitive) {
    size_t i;

    for (i = 0; pos + i < s_len; i++) {
        if (case_sensitive) {
            if (s[i] != s[i + pos]) {
                return 0;
            }
        } else {
            if (tolower(s[i]) != tolower(s[i + pos])) {
                return 0;
            }
        }
    }

    return 1;
}

size_t suffix_len(const char *s, const size_t s_len, const size_t pos, const int case_sensitive) {
    size_t i;

    for (i = 0; i < pos; i++) {
        if (case_sensitive) {
            if (s[pos - i] != s[s_len - i - 1]) {
                break;
            }
        } else {
            if (tolower(s[pos - i]) != tolower(s[s_len - i - 1])) {
                break;
            }
        }
    }

    return i;
}

void generate_find_skip(const char *find, const size_t f_len, size_t **skip_lookup, const int case_sensitive) {
    size_t i;
    size_t s_len;
    size_t *sl = ag_malloc(f_len * sizeof(size_t));
    *skip_lookup = sl;
    size_t last_prefix = f_len;

    for (i = last_prefix; i > 0; i--) {
        if (is_prefix(find, f_len, i, case_sensitive)) {
            last_prefix = i;
        }
        sl[i - 1] = last_prefix + (f_len - i);
    }

    for (i = 0; i < f_len; i++) {
        s_len = suffix_len(find, f_len, i, case_sensitive);
        if (find[i - s_len] != find[f_len - 1 - s_len]) {
            sl[f_len - 1 - s_len] = f_len - 1 - i + s_len;
        }
    }
}

size_t ag_max(size_t a, size_t b) {
    if (b > a) {
        return b;
    }
    return a;
}

/* Boyer-Moore strstr */
const char *boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len,
                                const size_t alpha_skip_lookup[], const size_t *find_skip_lookup) {
    ssize_t i;
    size_t pos = f_len - 1;

    while (pos < s_len) {
        for (i = f_len - 1; i >= 0 && s[pos] == find[i]; pos--, i--) {
        }
        if (i < 0) {
            return s + pos + 1;
        }
        pos += ag_max(alpha_skip_lookup[(unsigned char)s[pos]], find_skip_lookup[i]);
    }

    return NULL;
}

/* Copy-pasted from above. Yes I know this is bad. One day I might even fix it. */
const char *boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len,
                                    const size_t alpha_skip_lookup[], const size_t *find_skip_lookup) {
    ssize_t i;
    size_t pos = f_len - 1;

    while (pos < s_len) {
        for (i = f_len - 1; i >= 0 && tolower(s[pos]) == find[i]; pos--, i--) {
        }
        if (i < 0) {
            return s + pos + 1;
        }
        pos += ag_max(alpha_skip_lookup[(unsigned char)s[pos]], find_skip_lookup[i]);
    }

    return NULL;
}

strncmp_fp get_strstr(enum case_behavior casing) {
    strncmp_fp ag_strncmp_fp = &boyer_moore_strnstr;

    if (casing == CASE_INSENSITIVE) {
        ag_strncmp_fp = &boyer_moore_strncasestr;
    }

    return ag_strncmp_fp;
}

size_t invert_matches(const char *buf, const size_t buf_len, match_t matches[], size_t matches_len) {
    size_t i;
    size_t match_read_index = 0;
    size_t inverted_match_count = 0;
    size_t inverted_match_start = 0;
    size_t last_line_end = 0;
    int in_inverted_match = TRUE;
    match_t next_match;

    log_debug("Inverting %u matches.", matches_len);

    if (matches_len > 0) {
        next_match = matches[0];
    } else {
        next_match.start = buf_len + 1;
    }

    /* No matches, so the whole buffer is now a match. */
    if (matches_len == 0) {
        matches[0].start = 0;
        matches[0].end = buf_len - 1;
        return 1;
    }

    for (i = 0; i < buf_len; i++) {
        if (i == next_match.start) {
            i = next_match.end - 1;

            match_read_index++;

            if (match_read_index < matches_len) {
                next_match = matches[match_read_index];
            }

            if (in_inverted_match && last_line_end > inverted_match_start) {
                matches[inverted_match_count].start = inverted_match_start;
                matches[inverted_match_count].end = last_line_end - 1;

                inverted_match_count++;
            }

            in_inverted_match = FALSE;
        } else if (i == buf_len - 1 && in_inverted_match) {
            matches[inverted_match_count].start = inverted_match_start;
            matches[inverted_match_count].end = i;

            inverted_match_count++;
        } else if (buf[i] == '\n') {
            last_line_end = i + 1;

            if (!in_inverted_match) {
                inverted_match_start = last_line_end;
            }

            in_inverted_match = TRUE;
        }
    }

    for (i = 0; i < matches_len; i++) {
        log_debug("Inverted match %i start %i end %i.", i, matches[i].start, matches[i].end);
    }

    return inverted_match_count;
}

void realloc_matches(match_t **matches, size_t *matches_size, size_t matches_len) {
    if (matches_len < *matches_size) {
        return;
    }
    /* TODO: benchmark initial size of matches. 100 may be too small/big */
    *matches_size = *matches ? *matches_size * 2 : 100;
    *matches = ag_realloc(*matches, *matches_size * sizeof(match_t));
}

void compile_study(pcre **re, pcre_extra **re_extra, char *q, const int pcre_opts, const int study_opts) {
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;

    *re = pcre_compile(q, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (*re == NULL) {
        die("Bad regex! pcre_compile() failed at position %i: %s\nIf you meant to search for a literal string, run ag with -Q",
            pcre_err_offset,
            pcre_err);
    }
    *re_extra = pcre_study(*re, study_opts, &pcre_err);
    if (*re_extra == NULL) {
        log_debug("pcre_study returned nothing useful. Error: %s", pcre_err);
    }
}

/* This function is very hot. It's called on every file. */
int is_binary(const void *buf, const size_t buf_len) {
    size_t suspicious_bytes = 0;
    size_t total_bytes = buf_len > 512 ? 512 : buf_len;
    const unsigned char *buf_c = buf;
    size_t i;

    if (buf_len == 0) {
        return 0;
    }

    if (buf_len >= 3 && buf_c[0] == 0xEF && buf_c[1] == 0xBB && buf_c[2] == 0xBF) {
        /* UTF-8 BOM. This isn't binary. */
        return 0;
    }

    if (buf_len >= 5 && strncmp(buf, "%PDF-", 5) == 0) {
        /* PDF. This is binary. */
        return 1;
    }

    for (i = 0; i < total_bytes; i++) {
        if (buf_c[i] == '\0') {
            /* NULL char. It's binary */
            return 1;
        } else if ((buf_c[i] < 7 || buf_c[i] > 14) && (buf_c[i] < 32 || buf_c[i] > 127)) {
            /* UTF-8 detection */
            if (buf_c[i] > 193 && buf_c[i] < 224 && i + 1 < total_bytes) {
                i++;
                if (buf_c[i] > 127 && buf_c[i] < 192) {
                    continue;
                }
            } else if (buf_c[i] > 223 && buf_c[i] < 240 && i + 2 < total_bytes) {
                i++;
                if (buf_c[i] > 127 && buf_c[i] < 192 && buf_c[i + 1] > 127 && buf_c[i + 1] < 192) {
                    i++;
                    continue;
                }
            }
            suspicious_bytes++;
            /* Disk IO is so slow that it's worthwhile to do this calculation after every suspicious byte. */
            /* This is true even on a 1.6Ghz Atom with an Intel 320 SSD. */
            /* Read at least 32 bytes before making a decision */
            if (i >= 32 && (suspicious_bytes * 100) / total_bytes > 10) {
                return 1;
            }
        }
    }
    if ((suspicious_bytes * 100) / total_bytes > 10) {
        return 1;
    }

    return 0;
}

int is_regex(const char *query) {
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

int is_fnmatch(const char *filename) {
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

int binary_search(const char *needle, char **haystack, int start, int end) {
    int mid;
    int rc;

    if (start == end) {
        return -1;
    }

    mid = (start + end) / 2; /* can screw up on arrays with > 2 billion elements */

    rc = strcmp(needle, haystack[mid]);
    if (rc < 0) {
        return binary_search(needle, haystack, start, mid);
    } else if (rc > 0) {
        return binary_search(needle, haystack, mid + 1, end);
    }

    return mid;
}

static int wordchar_table[256];

void init_wordchar_table(void) {
    int i;
    for (i = 0; i < 256; ++i) {
        char ch = (char)i;
        wordchar_table[i] =
            ('a' <= ch && ch <= 'z') ||
            ('A' <= ch && ch <= 'Z') ||
            ('0' <= ch && ch <= '9') ||
            ch == '_';
    }
}

int is_wordchar(char ch) {
    return wordchar_table[(unsigned char)ch];
}

int is_lowercase(const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (!isascii(s[i]) || isupper(s[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

int is_directory(const char *path, const struct dirent *d) {
#ifdef HAVE_DIRENT_DTYPE
    /* Some filesystems, e.g. ReiserFS, always return a type DT_UNKNOWN from readdir or scandir. */
    /* Call stat if we don't find DT_DIR to get the information we need. */
    /* Also works for symbolic links to directories. */
    if (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK) {
        return d->d_type == DT_DIR;
    }
#endif
    char *full_path;
    struct stat s;
    ag_asprintf(&full_path, "%s/%s", path, d->d_name);
    if (stat(full_path, &s) != 0) {
        free(full_path);
        return FALSE;
    }
#ifdef _WIN32
    int is_dir = GetFileAttributesA(full_path) & FILE_ATTRIBUTE_DIRECTORY;
#else
    int is_dir = S_ISDIR(s.st_mode);
#endif
    free(full_path);
    return is_dir;
}

int is_symlink(const char *path, const struct dirent *d) {
#ifdef _WIN32
    char full_path[MAX_PATH + 1] = { 0 };
    sprintf(full_path, "%s\\%s", path, d->d_name);
    return (GetFileAttributesA(full_path) & FILE_ATTRIBUTE_REPARSE_POINT);
#else
#ifdef HAVE_DIRENT_DTYPE
    /* Some filesystems, e.g. ReiserFS, always return a type DT_UNKNOWN from readdir or scandir. */
    /* Call lstat if we find DT_UNKNOWN to get the information we need. */
    if (d->d_type != DT_UNKNOWN) {
        return (d->d_type == DT_LNK);
    }
#endif
    char *full_path;
    struct stat s;
    ag_asprintf(&full_path, "%s/%s", path, d->d_name);
    if (lstat(full_path, &s) != 0) {
        free(full_path);
        return FALSE;
    }
    free(full_path);
    return S_ISLNK(s.st_mode);
#endif
}

int is_named_pipe(const char *path, const struct dirent *d) {
#ifdef HAVE_DIRENT_DTYPE
    if (d->d_type != DT_UNKNOWN) {
        return d->d_type == DT_FIFO;
    }
#endif
    char *full_path;
    struct stat s;
    ag_asprintf(&full_path, "%s/%s", path, d->d_name);
    if (stat(full_path, &s) != 0) {
        free(full_path);
        return FALSE;
    }
    free(full_path);
    return S_ISFIFO(s.st_mode);
}

void ag_asprintf(char **ret, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (vasprintf(ret, fmt, args) == -1) {
        die("vasprintf returned -1");
    }
    va_end(args);
}

void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(LOG_LEVEL_ERR, fmt, args);
    va_end(args);
    exit(2);
}

#ifndef HAVE_FGETLN
char *fgetln(FILE *fp, size_t *lenp) {
    char *buf = NULL;
    int c, used = 0, len = 0;

    flockfile(fp);
    while ((c = getc_unlocked(fp)) != EOF) {
        if (!buf || len >= used) {
            size_t nsize;
            char *newbuf;
            nsize = used + BUFSIZ;
            if (!(newbuf = realloc(buf, nsize))) {
                funlockfile(fp);
                if (buf)
                    free(buf);
                return NULL;
            }
            buf = newbuf;
            used = nsize;
        }
        buf[len++] = c;
        if (c == '\n') {
            break;
        }
    }
    funlockfile(fp);
    *lenp = len;
    return buf;
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
        *n = len * 2;
    }

    memcpy(*lineptr, srcln, len);

#ifndef HAVE_FGETLN
    /* Our own implementation of fgetln() returns a malloc()d buffer that we
     * must free
     */
    free(srcln);
#endif

    (*lineptr)[len] = '\0';
    return len;
}
#endif

ssize_t buf_getline(const char **line, const char *buf, const size_t buf_len, const size_t buf_offset) {
    const char *cur = buf + buf_offset;
    ssize_t i;
    for (i = 0; cur[i] != '\n' && (buf_offset + i < buf_len); i++) {
    }
    *line = cur;
    return i;
}

#ifndef HAVE_REALPATH
/*
 * realpath() for Windows. Turns slashes into backslashes and calls _fullpath
 */
char *realpath(const char *path, char *resolved_path) {
    char *p;
    char tmp[_MAX_PATH + 1];
    strlcpy(tmp, path, sizeof(tmp));
    p = tmp;
    while (*p) {
        if (*p == '/') {
            *p = '\\';
        }
        p++;
    }
    return _fullpath(resolved_path, tmp, _MAX_PATH);
}
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size) {
    char *d = dst;
    const char *s = src;
    size_t n = size;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') {
                break;
            }
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (size != 0) {
            *d = '\0'; /* NUL-terminate dst */
        }

        while (*s++) {
        }
    }

    return (s - src - 1); /* count does not include NUL */
}
#endif

#ifndef HAVE_VASPRINTF
int vasprintf(char **ret, const char *fmt, va_list args) {
    int rv;
    *ret = NULL;
    va_list args2;
/* vsnprintf can destroy args, so we need to copy it for the second call */
#ifdef __va_copy
    /* non-standard macro, but usually exists */
    __va_copy(args2, args);
#elif va_copy
    /* C99 macro. We compile with -std=c89 but you never know */
    va_copy(args2, args);
#else
    /* Ancient compiler. This usually works but there are no guarantees. */
    memcpy(args2, args, sizeof(va_list));
#endif
    rv = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (rv < 0) {
        return rv;
    }
    *ret = malloc(++rv); /* vsnprintf doesn't count \0 */
    if (*ret == NULL) {
        return -1;
    }
    rv = vsnprintf(*ret, rv, fmt, args2);
    va_end(args2);
    if (rv < 0) {
        free(*ret);
    }
    return rv;
}
#endif
