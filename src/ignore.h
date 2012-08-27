#include <sys/dir.h>
#include <sys/types.h>

#ifndef IGNORE_H
#define IGNORE_H

#define SVN_DIR_PROP_BASE "/dir-prop-base"
#define SVN_DIR ".svn"
#define SVN_PROP_IGNORE "svn:ignore"

struct ignores {
    char **names; /* Non-regex ignore lines. Sorted so we can binary search them. */
    size_t names_len;
    char **regexes;
    size_t regexes_len;
    struct ignores *parent;
};
typedef struct ignores ignores;

ignores *root_ignores;

extern const char *evil_hardcoded_ignore_files[];
extern const char *ignore_pattern_files[];

void init_ignore(ignores *ig, ignores *parent);
void cleanup_ignore(ignores *ig);

void add_ignore_pattern(const char* pattern);

void cleanup_ignore_patterns();

void load_ignore_patterns(const char *ignore_filename);
void load_svn_ignore_patterns(const char *path, const int path_len);

int ackmate_dir_match(const char* dir_name);

int filename_filter(struct dirent *dir);
int filepath_filter(char *filepath);

#endif
