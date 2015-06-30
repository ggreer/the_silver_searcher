#ifndef IGNORE_H
#define IGNORE_H

#include <dirent.h>
#include <sys/types.h>

#define SVN_DIR_PROP_BASE "dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

struct ignores {
    char **extensions; /* File extensions to ignore */
    size_t extensions_len;
    size_t extensions_size;   /* alloc'd array size; size > len */

    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    size_t names_size;

    char **slash_names; /* Same but starts with a slash */
    size_t slash_names_len;
    size_t slash_names_size;

    char **regexes; /* For patterns that need fnmatch */
    size_t regexes_len;
    size_t regexes_size;

    char **slash_regexes;
    size_t slash_regexes_len;
    size_t slash_regexes_size;

    const char *dirname;
    size_t dirname_len;
    char *abs_path;
    size_t abs_path_len;

    struct ignores *parent;
};
typedef struct ignores ignores;

ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(ignores *ig, const char *pattern);

void load_ignore_patterns(ignores *ig, const char *path);
void load_svn_ignore_patterns(ignores *ig, const char *path);

int filename_filter(const char *path, const struct dirent *dir, void *baton);

int is_empty(ignores *ig);

#endif
