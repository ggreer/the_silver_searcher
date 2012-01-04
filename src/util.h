#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <ctype.h>

char* ag_strnstr(const char *s, const char *find, size_t slen);
char* ag_strncasestr(const char *s, const char *find, size_t slen);
int is_binary(const void* buf, const int buf_len);

#endif
