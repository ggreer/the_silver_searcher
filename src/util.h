#ifndef UTIL_H
#define UTIL_H

#include <dirent.h>
#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "config.h"
#include "log.h"
#include "options.h"

FILE *out_fd;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

void *ag_malloc(size_t size);
void *ag_realloc(void *ptr, size_t size);
void *ag_calloc(size_t nelem, size_t elsize);
char *ag_strdup(const char *s);
char *ag_strndup(const char *s, size_t size);

typedef struct {
    size_t start; /* Byte at which the match starts */
    size_t end;   /* and where it ends */
} match_t;

typedef struct {
    long total_bytes;
    long total_files;
    long total_matches;
    long total_file_matches;
    struct timeval time_start;
    struct timeval time_end;
} ag_stats;


ag_stats stats;

typedef const char *(*strncmp_fp)(const char *, const char *, const size_t, const size_t, const size_t[], const size_t *);

void free_strings(char **strs, const size_t strs_len);

void generate_alpha_skip(const char *find, size_t f_len, size_t skip_lookup[], const int case_sensitive);
int is_prefix(const char *s, const size_t s_len, const size_t pos, const int case_sensitive);
size_t suffix_len(const char *s, const size_t s_len, const size_t pos, const int case_sensitive);
void generate_find_skip(const char *find, const size_t f_len, size_t **skip_lookup, const int case_sensitive);

/* max is already defined on spec-violating compilers such as MinGW */
size_t ag_max(size_t a, size_t b);

const char *boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len,
                                const size_t alpha_skip_lookup[], const size_t *find_skip_lookup);
const char *boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len,
                                    const size_t alpha_skip_lookup[], const size_t *find_skip_lookup);

strncmp_fp get_strstr(enum case_behavior opts);

size_t invert_matches(const char *buf, const size_t buf_len, match_t matches[], size_t matches_len);
void realloc_matches(match_t **matches, size_t *matches_size, size_t matches_len);
void compile_study(pcre **re, pcre_extra **re_extra, char *q, const int pcre_opts, const int study_opts);


int is_binary(const void *buf, const size_t buf_len);
int is_regex(const char *query);
int is_fnmatch(const char *filename);
int binary_search(const char *needle, char **haystack, int start, int end);

void init_wordchar_table(void);
int is_wordchar(char ch);

int is_lowercase(const char *s);

int is_directory(const char *path, const struct dirent *d);
int is_symlink(const char *path, const struct dirent *d);
int is_named_pipe(const char *path, const struct dirent *d);

void die(const char *fmt, ...);

void ag_asprintf(char **ret, const char *fmt, ...);

ssize_t buf_getline(const char **line, const char *buf, const size_t buf_len, const size_t buf_offset);

#ifndef HAVE_FGETLN
char *fgetln(FILE *fp, size_t *lenp);
#endif
#ifndef HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
#ifndef HAVE_REALPATH
char *realpath(const char *path, char *resolved_path);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_VASPRINTF
int vasprintf(char **ret, const char *fmt, va_list args);
#endif

#endif
