#include <dirent.h>
#include <stdlib.h>

#include "scandir.h"
#include "util.h"

int ag_scandir(const char *dirname,
               struct dirent ***namelist,
               filter_fp filter,
               void *baton) {
    DIR *dirp = NULL;
    struct dirent **names = NULL;
    struct dirent *ents = NULL;

    size_t names_len = 32;
    size_t ents_bsz = 4000, ents_blen = 0;

    struct dirent *entry, *d;
    size_t dsz;
    size_t results_len = 0;

    dirp = opendir(dirname);
    if (dirp == NULL) {
        goto fail;
    }

    names = malloc(sizeof(struct dirent *) * names_len);
    if (names == NULL) {
        goto fail;
    }

    ents = malloc(ents_bsz);
    if (ents == NULL) {
        goto fail;
    }

    while ((entry = readdir(dirp)) != NULL) {
        fprintf(stderr, "\n");
        if ((*filter)(dirname, entry, baton) == FALSE) {
            continue;
        }

        if (results_len >= names_len) {
            struct dirent **old_names = names;
            names_len *= 2;
            names = realloc(names, sizeof(struct dirent *) * names_len);
            if (names == NULL) {
                free(old_names);
                goto fail;
            }
        }

        dsz = sizeof(struct dirent) + entry->d_reclen;

        if (ents_blen + dsz > ents_bsz) {
            struct dirent *old_ents = ents;
            while (ents_blen + dsz > ents_bsz) {
                ents_bsz *= 1.5;
            }
            ents = realloc(ents, ents_bsz);
            if (ents == NULL) {
                free(old_ents);
                goto fail;
            }
        }

        d = (struct dirent*) ((char*)ents + ents_blen);
        memcpy(d, entry, dsz);

        names[results_len] = (struct dirent*)ents_blen;
        ents_blen += dsz;

        results_len++;
    }

    size_t i;
    for (i = 0; i < results_len; i++) {
        size_t result_as_offset = (size_t) names[i];
        names[i] = (struct dirent*) ((char*)ents + result_as_offset);
    }

    if (results_len == 0) {
        names[0] = NULL;
        free(ents);
    }

    closedir(dirp);
    *namelist = names;
    return results_len;

fail:
    if (dirp) {
        closedir(dirp);
    }

    free(ents);
    free(names);
    return -1;
}
