#ifndef UTIL_H
#define UTIL_H

#include <string.h>

char* ag_strnstr(const char *s, const char *find, size_t slen, int (*strcmp_fp)(const char*, const char*, size_t));
int is_binary(const void* buf, const int buf_len);

#endif
