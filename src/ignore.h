#include <sys/dir.h>
#include <sys/types.h>

#ifndef IGNORE_H
#define IGNORE_H

void add_ignore_pattern(const char* pattern);

void cleanup_ignore_patterns();

void load_ignore_patterns(const char *ignore_filename);

int ignorefile_filter(struct dirent *dir);
int filename_filter(struct dirent *dir);

#endif
