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
    struct dirent *entry, *d;
    int names_len = 32;
    int results_len = 0;

    dirp = opendir(dirname);
    if (dirp == NULL) {
        goto fail;
    }

    names = malloc(sizeof(struct dirent *) * names_len);
    if (names == NULL) {
        goto fail;
    }

    while ((entry = readdir(dirp)) != NULL) {
        if ((*filter)(dirname, entry, baton) == FALSE) {
            continue;
        }
        if (results_len >= names_len) {
            struct dirent **tmp_names = names;
            names_len *= 2;
            names = realloc(names, sizeof(struct dirent *) * names_len);
            if (names == NULL) {
                free(tmp_names);
                goto fail;
            }
        }


#if defined HAS_MSVCLIBX
        size_t s_len = (entry->d_name - (char*)entry) + strlen(entry->d_name) + 1;
        d = (struct dirent *)malloc(s_len);
        if (d == NULL) goto fail;
        memcpy(d, entry, s_len);
#elif defined _MSC_VER
        size_t s_len = strlen(entry->d_name) + 1;
        d = (struct dirent *)malloc(sizeof(struct dirent) + s_len);
        char *s = (char*)d + sizeof(struct dirent);
        d->d_name = s;
#ifdef HAVE_DIRENT_DTYPE
        d->d_type = entry->d_type;
#endif /* HAVE_DIRENT_DTYPE */
        memcpy(s, entry->d_name, s_len);
#else

#if defined(__MINGW32__) || defined(__CYGWIN__)
        d = malloc(sizeof(struct dirent));
#else
        d = malloc(entry->d_reclen);
#endif /* defined(__MINGW32__) || defined(__CYGWIN__) */

        if (d == NULL) {
            goto fail;
        }
#if defined(__MINGW32__) || defined(__CYGWIN__)
        memcpy(d, entry, sizeof(struct dirent));
#else
        memcpy(d, entry, entry->d_reclen);
#endif /* defined(__MINGW32__) || defined(__CYGWIN__) */


#endif /* _MSC_VER */

        names[results_len] = d;
        results_len++;
    }

    closedir(dirp);
    *namelist = names;
    return results_len;

fail:
    if (dirp) {
        closedir(dirp);
    }

    if (names != NULL) {
        int i;
        for (i = 0; i < results_len; i++) {
            free(names[i]);
        }
        free(names);
    }
    return -1;
}
