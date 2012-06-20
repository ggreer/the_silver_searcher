#include <sys/dir.h>
#include <sys/types.h>

#ifndef IGNORE_H
#define IGNORE_H

#define SVN_DIR_PROP_BASE "/dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

void add_ignore_pattern(const char* pattern);

void cleanup_ignore_patterns();

void load_ignore_patterns(const char *ignore_filename);
void load_svn_ignore_patterns(const char *path, const int path_len);

int ignorefile_filter(struct dirent *dir);
int filename_filter(struct dirent *dir);

#endif
