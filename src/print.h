#ifndef PRINT_H
#define PRINT_H

#include "util.h"

void print_init_context(void);
void print_cleanup_context(void);
void print_context_append(const char *line, size_t len);
void print_trailing_context(const char *path, const char *buf, size_t n);
void print_path(const char *path, const char sep);
void print_path_count(const char *path, const char sep, const size_t count);
void print_line(const char *buf, size_t buf_pos, size_t prev_line_offset);
void print_binary_file_matches(const char *path);
void print_file_matches(const char *path, const char *buf, const size_t buf_len, const match_t matches[], const size_t matches_len);
void print_line_number(size_t line, const char sep);
void print_column_number(const match_t matches[], size_t last_printed_match,
                         size_t prev_line_offset, const char sep);
void print_file_separator(void);
const char *normalize_path(const char *path);

#ifdef _WIN32
void windows_use_ansi(int use_ansi);
int fprintf_w32(FILE *fp, const char *format, ...);
int cyg_isatty(int fd);
#define ag_isatty(fd) (isatty(fd) || cyg_isatty(fd))
#else
#define ag_isatty isatty
#endif

#endif
