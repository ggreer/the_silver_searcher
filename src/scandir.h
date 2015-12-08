#ifndef SCANDIR_H
#define SCANDIR_H

#include "ignore.h"

struct ignore_iters {
    size_t names_i;
    size_t slash_names_i;
    size_t globs_i;
    size_t slash_globs_i;
    struct ignore_iters *parent;
};

typedef struct {
    const ignores *ig;
    struct ignore_iters iters;
    const char *base_path;
    size_t base_path_len;
} scandir_baton_t;

typedef int (*filter_fp)(const char *path, const struct dirent *, void *);

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton);

#endif
