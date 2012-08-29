#include <dirent.h>
#include <stdlib.h>

#include "scandir.h"
#include "util.h"

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton
              ) {
    DIR *dirp = NULL;
    struct dirent **names = NULL;
    struct dirent *entry, *d;
    int names_len = 32;
    int results_len = 0;

    dirp = opendir(dirname);
    if (dirp == NULL) {
        goto fail;
    }

    names = malloc(sizeof(struct dirent*) * names_len);
    if (names == NULL) {
        goto fail;
    }

    while ((entry = readdir(dirp)) != NULL) {
        if ((*filter)(entry, baton) == FALSE) {
            continue;
        }
        if (results_len >= names_len) {
            names_len = names_len * 2;
            names = realloc(names, sizeof(struct dirent*) * names_len);
            if (names == NULL) {
                goto fail;
            }
        }
        d = malloc(sizeof(struct dirent));
        if (d == NULL) {
            goto fail;
        }
        memcpy(d, entry, sizeof(struct dirent));
        names[results_len] = d;
        results_len++;
    }

    closedir(dirp);
    *namelist = names;
    return results_len;

    fail:;
    int i;
    if (dirp) {
        closedir(dirp);
    }

    if (names != NULL) {
        for (i = 0; i < results_len; i++) {
            free(names[i]);
        }
        free(names);
    }
    return -1;
}
