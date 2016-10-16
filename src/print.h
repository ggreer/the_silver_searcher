#ifndef PRINT_H
#define PRINT_H

#include "util.h"

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

pthread_mutex_t print_mtx;

const char *normalize_path(const char *path);

void print_path(const char *path, const char sep);
void print_path_count(const char *path, const char sep, const size_t count);
void print_line_number(size_t line, const char sep);
void print_column_number(const match_t matches[], size_t last_printed_match,
                         size_t prev_line_offset, const char sep);
void print_line(const char *buf, size_t buf_pos, size_t prev_line_offset);
void print_file_separator(void);
void print_binary_file_matches(const char *path);
void print_file_matches(const char *path, const char *buf, const size_t buf_len, const match_t matches[], const size_t matches_len);

void print_results(const char *buf, const size_t buf_len, const char *dir_full_path, search_results_t *sr);

#ifdef _WIN32
void windows_use_ansi(int use_ansi);
int fprintf_w32(FILE *fp, const char *format, ...);
#endif

#endif
