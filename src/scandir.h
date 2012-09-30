#ifndef SCANDIR_H
#define SCANDIR_H

typedef int (*filter_fp)(const char *path, const struct dirent *, void *);

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton
              );

#endif
