#ifndef IGNORE_H
#define IGNORE_H

#include <dirent.h>
#include <pcre.h>
#include <sys/types.h>

#define SVN_DIR_PROP_BASE "dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

#define HGIGNORE ".hgignore"

#define IGNORE_FLAG_INVERT 1
#define IGNORE_FLAG_ISDIR  2

struct ignores {
    pcre **regexes; /* For patterns that need fnmatch */
    pcre_extra **extra;
    int *flags;
    size_t regexes_len;
    struct ignores *parent;
};
typedef struct ignores ignores;

ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

ignores *init_ignore(ignores *parent);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(ignores *ig, const char* pattern);

void load_ignore_patterns(ignores *ig, const char *path);

void load_hg_ignore_patterns(ignores *ig, const char *path);

void load_svn_ignore_patterns(ignores *ig, const char *path);

int filename_filter(const char *path, const struct dirent *dir, void *baton);

#endif
