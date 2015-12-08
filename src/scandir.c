#include <dirent.h>
#include <stdlib.h>

#include "scandir.h"
#include "util.h"

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton) {
    struct dirent **results = NULL;
    struct dirent **scanned_names = NULL;
    int i, scanned_names_len, results_len = 0;

    scanned_names_len = scandir(dirname, &scanned_names, 0, alphasort);
    if (scanned_names_len == -1) {
        goto fail;
    }

    results = malloc(sizeof(struct dirent *) * scanned_names_len);
    if (results == NULL) {
        goto fail;
    }

    for (i = 0; i < scanned_names_len; i++) {
        if ((*filter)(dirname, scanned_names[i], baton) == FALSE) {
            free(scanned_names[i]);
            scanned_names[i] = NULL;
            continue;
        }
        results[results_len] = scanned_names[i];
        results_len++;
    }

    *namelist = results;
    return results_len;

fail:
    if (results != NULL) {
        for (i = 0; i < results_len; i++) {
            free(results[i]);
        }
        free(results);
    }
    if (scanned_names != NULL) {
        for (i = 0; i < scanned_names_len; i++) {
            free (scanned_names[i]);
        }
    }
    return -1;
}
