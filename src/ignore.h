#ifndef IGNORE_H
#define IGNORE_H

#include <dirent.h>
#include <sys/types.h>

struct ignores {
    char **extensions; /* File extensions to ignore */
    size_t extensions_len;

    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    char **slash_names; /* Same but starts with a slash */
    size_t slash_names_len;

    char **regexes; /* For patterns that need fnmatch */
    size_t regexes_len;
    char **invert_regexes; /* For "!" patterns */
    size_t invert_regexes_len;
    char **slash_regexes;
    size_t slash_regexes_len;

    const char *dirname;
    size_t dirname_len;
    char *abs_path;
    size_t abs_path_len;

    struct ignores *parent;
};
typedef struct ignores ignores;

extern ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(ignores *ig, const char *pattern);

void load_ignore_patterns(ignores *ig, const char *path);

int filename_filter(const char *path, const struct dirent *dir, void *baton);

int is_empty(ignores *ig);

int path_ignore_search(const ignores *ig, const char *path, const char *filename);

#endif
