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

    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    char **slash_names; /* Same but starts with a slash */
    size_t slash_names_len;

    char **partial_names; /* Partial matches from the parent directory. */
    size_t partial_names_len;
    char **partial_slash_names;
    size_t partial_slash_names_len;

    char **globs; /* For patterns that need fnmatch */
    size_t globs_len;
    char **slash_globs;
    size_t slash_globs_len;

    char **partial_globs;
    size_t partial_globs_len;
    char **partial_slash_globs;
    size_t partial_slash_globs_len;

    const char *dirname;
    size_t dirname_len;
    char *abs_path;
    size_t abs_path_len;

    struct ignores *parent;
};
typedef struct ignores ignores;
struct ag_dirent;

ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len, struct ag_dirent *partials);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(ignores *ig, const char *pattern);

void load_ignore_patterns(ignores *ig, const char *path);
void load_svn_ignore_patterns(ignores *ig, const char *path);

int filename_filter(const char *path, struct ag_dirent *ag_dir, void *baton);

int is_empty(ignores *ig);

#endif
