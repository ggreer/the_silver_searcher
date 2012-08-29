#ifndef IGNORE_H
#define IGNORE_H

#include <sys/dir.h>
#include <sys/types.h>

#define SVN_DIR_PROP_BASE "/dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

struct ignores {
    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    char **regexes; /* For patterns that need fnmatch */
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

void load_ignore_patterns(ignores *ig, const char *ignore_filename);
void load_svn_ignore_patterns(ignores *ig, const char *path);

int ackmate_dir_match(const char* dir_name);

int filename_filter(const struct dirent *dir, void *baton);

#endif
