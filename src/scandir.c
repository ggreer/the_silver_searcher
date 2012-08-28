#include <dirent.h>
#include <stdlib.h>

#include "scandir.h"
#include "util.h"

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton
              ) {
    DIR *dirp;
    struct dirent **names;
    struct dirent *entry, *d;
    int names_len = 32;
    int results_len = 0;

    dirp = opendir(dirname);
    if (dirp == NULL) {
        return -1;
    }

    /* TODO: handle allocation failures */
    names = malloc(sizeof(struct dirent*) * names_len);

    while ((entry = readdir(dirp)) != NULL) {
        if ((*filter)(entry, baton) == FALSE) {
            continue;
        }
        if (results_len >= names_len) {
            /* TODO: handle errors here */
            names_len = names_len * 2;
            names = realloc(names, sizeof(struct dirent*) * names_len);
        }
        d = malloc(sizeof(struct dirent));
        memcpy(d, entry, sizeof(struct dirent));
        names[results_len] = d;
        results_len++;
    }

    closedir(dirp);
    *namelist = names;
    return results_len;
}
