#ifndef IGNORE_H
#define IGNORE_H

#include <dirent.h>
#include <sys/types.h>

#define SVN_DIR_PROP_BASE "dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

enum pattern_type {
    PT_extension,
    PT_regex,
    PT_slash_regex,
    PT_name,
    PT_slash_name,
    PT_LAST
};

enum pattern_action {
    PA_include, /* keep file in the to search list */
    PA_exclude, /* remove file from the to search list */
    PA_LAST
};

struct gitignore {
    char *pattern;
    size_t len;
    char *prepend; //!< This should be prepended to a file to match this pattern. This is used to make sure parent gitignores work properly
    size_t prepend_len;
    enum pattern_type type;
    enum pattern_action action; /* What to do when this pattern matches. It can be either include the
                                   file 1 or exclude the file 0 */
};

struct ignores {
    char **extensions; /* File extensions to ignore */
    size_t extensions_len;

    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    char **slash_names; /* Same but starts with a slash */
    size_t slash_names_len;

    char **regexes; /* For patterns that need fnmatch */
    size_t regexes_len;
    char **slash_regexes;
    size_t slash_regexes_len;

    const char *dirname;
    size_t dirname_len;
    char *abs_path;
    size_t abs_path_len;

    struct gitignore **gitignores;
    size_t gitignores_len;

    struct ignores *parent;
};
typedef struct ignores ignores;

ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];
extern const char *gitignore_pattern_files[];

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(ignores *ig, const char *pattern);

void load_ignore_patterns(ignores *ig, const char *path);
void load_git_ignore_patterns(ignores *ig, const char *path, char * path_to_pwd);
void load_svn_ignore_patterns(ignores *ig, const char *path);

int filename_filter(const char *path, const struct dirent *dir, void *baton);

int is_empty(ignores *ig);

#endif
