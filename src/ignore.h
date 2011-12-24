#include <sys/dir.h>
#include <sys/types.h>

#ifndef IGNORE_H
#define IGNORE_H

void add_ignore_pattern(const char* pattern);

void cleanup_ignore_patterns();

void load_ignore_patterns(const char *ignore_filename);

int ignorefile_filter(const struct dirent *dir);
int filename_filter(const struct dirent *dir);

#endif
