#ifndef PRINT_H
#define PRINT_H

#include "util.h"

void print_path(const char *path, const char sep);
void print_binary_file_matches(const char *path);
void print_file_matches(const char *path, const char *buf, const size_t buf_len, const match_t matches[], const size_t matches_len);
void print_line_number(size_t line, const char sep);
void print_file_separator(void);
const char *normalize_path(const char *path);

#endif
