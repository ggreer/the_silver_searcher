#include <dirent.h>
#include <stdlib.h>

#include "scandir.h"
#include "util.h"

int ag_scandir(const char *dirname,
               struct ag_dirent ***namelist,
               filter_fp filter,
               void *baton) {
    struct ag_dirent **results = NULL;
    struct dirent **scanned_names = NULL;
    int i, scanned_names_len, results_len = 0;

    scanned_names_len = scandir(dirname, &scanned_names, 0, alphasort);
    if (scanned_names_len == -1) {
        goto fail;
    }

    results = malloc(sizeof(struct ag_dirent *) * scanned_names_len);
    if (results == NULL) {
        goto fail;
    }

    for (i = 0; i < scanned_names_len; i++) {
        struct ag_dirent *ag_dir = malloc(sizeof(struct ag_dirent));
        if (ag_dir == NULL) {
            goto fail;
        }
        *ag_dir = (struct ag_dirent){ scanned_names[i], NULL, 0, NULL, 0 };
        int result = (*filter)(dirname, ag_dir, baton);
        if (result == -1) {
            goto fail;
        } else if (result == 0) {
            free(scanned_names[i]);
            free(ag_dir->partial_name_matches);
            free(ag_dir->partial_glob_matches);
            free(ag_dir);
            scanned_names[i] = NULL;
            continue;
        }
        results[results_len] = ag_dir;
        results_len++;
    }

    free(scanned_names);
    *namelist = results;
    return results_len;

fail:
    if (results != NULL) {
        for (i = 0; i < results_len; i++) {
            if (results[i] != NULL) {
                free(results[i]->partial_name_matches);
                free(results[i]->partial_glob_matches);
                free(results[i]);
            }
        }
        free(results);
    }
    if (scanned_names != NULL) {
        for (i = 0; i < scanned_names_len; i++) {
            free(scanned_names[i]);
        }
    }
    free(scanned_names);
    return -1;
}
