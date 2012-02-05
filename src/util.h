#ifndef UTIL_H
#define UTIL_H

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

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[], int case_sensitive);
char* boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]);
char* boyer_moore_strncasestr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]);

void invert_matches(match matches[], int matches_len, const int buf_len);

int is_binary(const void* buf, const int buf_len);
int is_regex(const char* query, const int query_len);

#ifndef HAVE_STRLCAT
size_t strlcat(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size);
#endif

#endif
