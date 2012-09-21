#ifndef UTIL_H
#define UTIL_H

#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "config.h"
#include "log.h"
#include "options.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

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

typedef char *(*strncmp_fp)(const char*, const char*, const size_t, const size_t, const size_t[]);

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[], int case_sensitive);

char* boyer_moore_strnstr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]);
char* boyer_moore_strncasestr(const char *s, const char *find, const size_t s_len, const size_t f_len, const size_t skip_lookup[]);

strncmp_fp get_strstr(cli_options opts);

int invert_matches(match matches[], int matches_len, const int buf_len);
void build_word_regex();
void compile_study(pcre **re, pcre_extra **re_extra, char *q, const int pcre_opts, const int study_opts);

int is_binary(const void* buf, const int buf_len);
int is_regex(const char* query);
int is_fnmatch(const char* filename);
int binary_search(const char* needle, char **haystack, int start, int end);

int is_whitespace(const char ch);

int contains_uppercase(const char* s);

#ifndef HAVE_STRLCAT
size_t strlcat(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
#ifndef HAVE_STRNDUP
char * strndup (const char *s, size_t n);
#endif

#endif
