#ifndef UTIL_H
#define UTIL_H

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[]);
char* boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]);
char* boyer_moore_strncasestr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]);

int is_binary(const void* buf, const int buf_len);
int is_regex(const char* query, const int query_len);

#ifndef HAVE_STRLCAT
size_t strlcat(char *dest, const char *src, size_t size);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t size);
#endif

#endif
