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

void check_err(int err, const char *action);

typedef struct {
    int start; /* Byte at which the match starts */
    int end; /* and where it ends */
} match;

typedef struct {
    long total_bytes;
    long total_files;
    long total_matches;
    struct timeval time_start;
    struct timeval time_end;
} ag_stats;

ag_stats stats;

typedef const char *(*strncmp_fp)(const char*, const char*, const size_t, const size_t, const size_t[]);

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[], int case_sensitive);

const char* boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]);
const char* boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]);

strncmp_fp get_strstr(cli_options opts);

int invert_matches(match matches[], int matches_len, const int buf_len);
void compile_study(pcre **re, pcre_extra **re_extra, char *q, const int pcre_opts, const int study_opts);

int is_binary(const void* buf, const int buf_len);
int is_regex(const char* query);
int is_fnmatch(const char* filename);
int binary_search(const char* needle, char **haystack, int start, int end);

void init_wordchar_table(void);
int is_wordchar(char ch);

int contains_uppercase(const char* s);

int is_directory(const char *path, const struct dirent *d);
int is_symlink(const char *path, const struct dirent *d);

#ifndef HAVE_FGETLN
char * fgetln(FILE *fp, size_t *lenp);
#endif
#ifndef HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
#ifndef HAVE_STRNDUP
char * strndup (const char *s, size_t n);
#endif
#ifndef HAVE_ASPRINTF
int asprintf(char **ret, const char *format, ...);
#endif

#endif
