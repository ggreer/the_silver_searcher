#ifndef UTIL_H
#define UTIL_H

char* ag_strnstr(const char *s, const char *find, size_t slen);
char* ag_strncasestr(const char *s, const char *find, size_t slen);
int is_binary(const void* buf, const int buf_len);

#endif
