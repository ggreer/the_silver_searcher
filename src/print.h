#ifndef PRINT_H
#define PRINT_H

typedef struct {
    int start; // Byte at which the match starts
    int end; // and where it ends
} match;

void print_path(const char* path);

void print_file_matches(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len);
void print_file_matches_with_context(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len);

#endif
