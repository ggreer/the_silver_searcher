#ifndef SCANDIR_H
#define SCANDIR_H

#include "ignore.h"

struct ignore_iters {
    size_t names_i;
    size_t slash_names_i;
    size_t partial_names_i;
    struct ignore_iters *parent;
};

struct ag_dirent {
    struct dirent *dirent;
    char **partial_name_matches;
    size_t partial_name_matches_len;
    char **partial_glob_matches;
    size_t partial_glob_matches_len;
};

typedef struct {
    const ignores *ig;
    struct ignore_iters iters;
    const char *base_path;
    size_t base_path_len;
} scandir_baton_t;

typedef int (*filter_fp)(const char *path, struct ag_dirent *, void *);

int ag_scandir(const char *dirname,
               struct ag_dirent ***namelist,
               filter_fp filter,
               void *baton);

#endif
