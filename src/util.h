#ifndef UTIL_H
#define UTIL_H

void generate_skip_lookup(const char *find, size_t f_len, size_t skip_lookup[]);
char* boyer_moore_strnstr(const char *s, const char *find, size_t s_len, size_t f_len, size_t skip_lookup[]);

int is_binary(const void* buf, const int buf_len);

size_t ag_strlcat(char *dest, const char *src, size_t size);
size_t ag_strlcpy(char *dest, const char *src, size_t size);

#endif
