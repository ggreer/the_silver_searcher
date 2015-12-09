#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "scandir.h"
#include "util.h"

#ifdef _WIN32
#include <shlwapi.h>
#define fnmatch(x, y, z) (!PathMatchSpec(y, x))
#else
#include <fnmatch.h>
const int fnmatch_flags = FNM_PATHNAME;
#endif

/* TODO: build a huge-ass list of files we want to ignore by default (build cache stuff, pyc files, etc) */

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

/* Warning: changing the first string will break skip_vcs_ignores. */
const char *ignore_pattern_files[] = {
    ".agignore",
    ".gitignore",
    ".git/info/exclude",
    ".hgignore",
    ".svn",
    NULL
};

int is_empty(ignores *ig) {
    return (ig->extensions_len + ig->names_len + ig->slash_names_len + ig->partial_names_len + ig->partial_slash_names_len + ig->globs_len + ig->slash_globs_len + ig->partial_globs_len + ig->partial_slash_globs_len == 0);
};

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len, struct ag_dirent *partials) {
    ignores *ig = ag_malloc(sizeof(ignores));

    ig->extensions = NULL;
    ig->extensions_len = 0;

    ig->names = NULL;
    ig->names_len = 0;
    ig->slash_names = NULL;
    ig->slash_names_len = 0;

    ig->globs = NULL;
    ig->globs_len = 0;
    ig->slash_globs = NULL;
    ig->slash_globs_len = 0;

    ig->regexes = NULL;
    ig->regexes_len = 0;
    ig->slash_regexes = NULL;
    ig->slash_regexes_len = 0;

    ig->dirname = dirname;
    ig->dirname_len = dirname_len;

    if (partials == NULL) {
        ig->partial_names = NULL;
        ig->partial_names_len = 0;
        ig->partial_globs = NULL;
        ig->partial_globs_len = 0;
    } else {
        ig->partial_names = partials->partial_name_matches;
        ig->partial_names_len = partials->partial_name_matches_len;
        ig->partial_globs = partials->partial_glob_matches;
        ig->partial_globs_len = partials->partial_glob_matches_len;
    }

    if (parent) {
        while (is_empty(parent) && parent->parent) {
            parent = parent->parent;
        }
    }
    ig->parent = parent;

    if (parent && parent->abs_path_len > 0) {
        ag_asprintf(&(ig->abs_path), "%s/%s", parent->abs_path, dirname);
        ig->abs_path_len = parent->abs_path_len + 1 + dirname_len;
    } else if (dirname_len == 1 && dirname[0] == '.') {
        ig->abs_path = ag_malloc(sizeof(char));
        ig->abs_path[0] = '\0';
        ig->abs_path_len = 0;
    } else {
        ag_asprintf(&(ig->abs_path), "%s", dirname);
        ig->abs_path_len = dirname_len;
    }
    return ig;
}

void cleanup_ignore(ignores *ig) {
    if (ig == NULL) {
        return;
    }
    free_strings(ig->extensions, ig->extensions_len);
    free_strings(ig->names, ig->names_len);
    free_strings(ig->slash_names, ig->slash_names_len);
    free_strings(ig->globs, ig->globs_len);
    free_strings(ig->slash_globs, ig->slash_globs_len);
    if (ig->abs_path) {
        free(ig->abs_path);
    }
    free(ig);
}

void add_ignore_pattern(ignores *ig, const char *pattern) {
    int pattern_len;

    /* Strip off the leading dot so that matches are more likely. */
    if (strncmp(pattern, "./", 2) == 0) {
        pattern++;
    }

    /* Kill trailing whitespace */
    for (pattern_len = strlen(pattern); pattern_len > 0; pattern_len--) {
        if (!isspace(pattern[pattern_len - 1])) {
            break;
        }
    }

    if (pattern_len == 0) {
        log_debug("Pattern is empty. Not adding any ignores.");
        return;
    }

    char ***patterns_p;
    size_t *patterns_len;
    if (is_fnmatch(pattern)) {
        if (pattern[0] == '*' && pattern[1] == '.' && !(is_fnmatch(pattern + 2))) {
            patterns_p = &(ig->extensions);
            patterns_len = &(ig->extensions_len);
            pattern += 2;
        } else if (pattern[0] == '/') {
            patterns_p = &(ig->slash_globs);
            patterns_len = &(ig->slash_globs_len);
            pattern++;
            pattern_len--;
        } else {
            patterns_p = &(ig->globs);
            patterns_len = &(ig->globs_len);
        }
    } else {
        if (pattern[0] == '/') {
            patterns_p = &(ig->slash_names);
            patterns_len = &(ig->slash_names_len);
            pattern++;
            pattern_len--;
        } else {
            patterns_p = &(ig->names);
            patterns_len = &(ig->names_len);
        }
    }

    ag_insert_str_sorted(patterns_p, patterns_len, pattern, pattern_len);
    log_debug("added ignore pattern %s to %s", pattern,
              ig == root_ignores ? "root ignores" : ig->abs_path);
}

/* For loading git/hg ignore patterns */
void load_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s: not readable", path);
        return;
    }
    log_debug("Loading ignore file %s.", path);

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(ig, line);
    }

    free(line);
    fclose(fp);
}

void load_svn_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    char *dir_prop_base;
    ag_asprintf(&dir_prop_base, "%s/%s", path, SVN_DIR_PROP_BASE);

    fp = fopen(dir_prop_base, "r");
    if (fp == NULL) {
        log_debug("Skipping svn ignore file %s", dir_prop_base);
        free(dir_prop_base);
        return;
    }

    char *entry = NULL;
    size_t entry_len = 0;
    char *key = ag_malloc(32); /* Sane start for max key length. */
    size_t key_len = 0;
    size_t bytes_read = 0;
    char *entry_line;
    size_t line_len;
    int matches;

    while (fscanf(fp, "K %zu\n", &key_len) == 1) {
        key = ag_realloc(key, key_len + 1);
        bytes_read = fread(key, 1, key_len, fp);
        key[key_len] = '\0';
        matches = fscanf(fp, "\nV %zu\n", &entry_len);
        if (matches != 1) {
            log_debug("Unable to parse svnignore file %s: fscanf() got %i matches, expected 1.", dir_prop_base, matches);
            goto cleanup;
        }

        if (strncmp(SVN_PROP_IGNORE, key, bytes_read) != 0) {
            log_debug("key is %s, not %s. skipping %u bytes", key, SVN_PROP_IGNORE, entry_len);
            /* Not the key we care about. fseek and repeat */
            fseek(fp, entry_len + 1, SEEK_CUR); /* +1 to account for newline. yes I know this is hacky */
            continue;
        }
        /* Aww yeah. Time to ignore stuff */
        entry = ag_malloc(entry_len + 1);
        bytes_read = fread(entry, 1, entry_len, fp);
        entry[bytes_read] = '\0';
        log_debug("entry: %s", entry);
        break;
    }
    if (entry == NULL) {
        goto cleanup;
    }
    char *patterns = entry;
    size_t patterns_len = strlen(patterns);
    while (*patterns != '\0' && patterns < (entry + bytes_read)) {
        for (line_len = 0; line_len < patterns_len; line_len++) {
            if (patterns[line_len] == '\n') {
                break;
            }
        }
        if (line_len > 0) {
            entry_line = ag_strndup(patterns, line_len);
            add_ignore_pattern(ig, entry_line);
            free(entry_line);
        }
        patterns += line_len + 1;
        patterns_len -= line_len + 1;
    }
    free(entry);
cleanup:
    free(dir_prop_base);
    free(key);
    fclose(fp);
}

static int ackmate_dir_match(const char *dir_name) {
    if (opts.ackmate_dir_filter == NULL) {
        return 0;
    }
    /* we just care about the match, not where the matches are */
    return pcre_exec(opts.ackmate_dir_filter, NULL, dir_name, strlen(dir_name), 0, 0, NULL, 0);
}

/* This is the hottest code in Ag. 10-15% of all execution time is spent here */
static int path_ignore_search(const ignores *ig, const char *path, const char *filename, struct ignore_iters *iters, int search_slashes, struct ag_dirent *ag_dir) {
    char *temp;
    size_t i;
    int match_pos;

    match_pos = binary_search(filename, ig->names, 0, ig->names_len);
    if (match_pos >= 0) {
        log_debug("file %s ignored because name matches static pattern %s", filename, ig->names[match_pos]);
        return 1;
    }

    ag_asprintf(&temp, "%s/%s", path[0] == '.' ? path + 2 : path, filename);

    if (strncmp(temp, ig->abs_path, ig->abs_path_len) == 0) {
        char *slash_filename = temp + ig->abs_path_len;
        if (slash_filename[0] == '/') {
            slash_filename++;
        }

        for (; iters->names_i < ig->names_len; iters->names_i++) {
            int cmp = cmp_leading_dir(slash_filename, ig->names[iters->names_i]);
            if (cmp == 0) {
                log_debug("file %s ignored because name matches static pattern %s", temp, ig->names[iters->names_i]);
                free(temp);
                return 1;
            } else if (cmp == 2) {
                const char *subdir = ig->names[iters->names_i];
                size_t subdir_len = ag_subdir(&subdir);
                log_debug("file %s's subdirectories may match static pattern %s; adding subdir %s to child's ignores", temp, ig->names[iters->names_i], subdir);
                ag_insert_str_sorted(&ag_dir->partial_name_matches, &ag_dir->partial_name_matches_len, subdir, subdir_len);
            } else {
                log_debug("file %s doesn't match static pattern %s at index %d", temp, ig->names[iters->names_i], iters->names_i);
            }
            if (cmp < 0) {
                break;
            }
        }

        if (search_slashes) {
            for (; iters->slash_names_i < ig->slash_names_len; iters->slash_names_i++) {
                int cmp = cmp_leading_dir(slash_filename, ig->slash_names[iters->slash_names_i]);
                if (cmp == 0) {
                    log_debug("file %s ignored because name matches slash static pattern %s", temp, ig->slash_names[iters->slash_names_i]);
                    free(temp);
                    return 1;
                } else if (cmp == 2) {
                    const char *subdir = ig->slash_names[iters->slash_names_i];
                    size_t subdir_len = ag_subdir(&subdir);
                    log_debug("file %s's subdirectories may match slash static pattern %s; adding subdir %s to child's ignores", temp, ig->slash_names[iters->slash_names_i], subdir);
                    ag_insert_str_sorted(&ag_dir->partial_name_matches, &ag_dir->partial_name_matches_len, subdir, subdir_len);
                } else {
                    log_debug("file %s doesn't match slash static pattern %s at index %d", temp, ig->slash_names[iters->slash_names_i], iters->slash_names_i);
                }
                if (cmp < 0) {
                    break;
                }
            }

            for (; iters->partial_names_i < ig->partial_names_len; iters->partial_names_i++) {
                int cmp = cmp_leading_dir(slash_filename, ig->partial_names[iters->partial_names_i]);
                if (cmp == 0) {
                    log_debug("file %s ignored because name matches partial static pattern %s", temp, ig->partial_names[iters->partial_names_i]);
                    free(temp);
                    return 1;
                } else if (cmp == 2) {
                    const char *subdir = ig->partial_names[iters->partial_names_i];
                    size_t subdir_len = ag_subdir(&subdir);
                    log_debug("file %s's subdirectories may match partial static pattern %s; adding subdir %s to child's ignores", temp, ig->partial_names[iters->partial_names_i], subdir);
                    ag_insert_str_sorted(&ag_dir->partial_name_matches, &ag_dir->partial_name_matches_len, subdir, subdir_len);
                } else {
                    log_debug("file %s doesn't match partial static pattern %s at index %d", temp, ig->partial_names[iters->partial_names_i], iters->partial_names_i);
                }
                if (cmp < 0) {
                    break;
                }
            }

            for (; iters->slash_globs_i < ig->slash_globs_len; iters->slash_globs_i++) {
                /* cmp_leading_dir_glob modifies slash_filename, but restores it before it returns,
                 * so this should be safe. */
                int cmp = cmp_leading_dir_glob((char *)slash_filename, ig->slash_globs[iters->slash_globs_i]);
                if (cmp == 0) {
                    log_debug("file %s ignored because name matches slash glob pattern %s", slash_filename, ig->slash_globs[iters->slash_globs_i]);
                    free(temp);
                    return 1;
                } else if (cmp == 2) {
                    const char *subdir = ig->slash_globs[iters->slash_globs_i];
                    size_t subdir_len = ag_subdir(&subdir);
                    log_debug("file %s's subdirectories may match slash glob pattern %s; adding subdir %s to child's ignores", temp, ig->slash_globs[iters->slash_globs_i], subdir);
                    ag_insert_str_sorted(&ag_dir->partial_glob_matches, &ag_dir->partial_glob_matches_len, subdir, subdir_len);
                } else {
                    log_debug("pattern %s doesn't match slash file %s", ig->slash_globs[iters->slash_globs_i], slash_filename);
                }
            }
        }
    }

    for (i = 0; i < ig->globs_len; i++) {
        int cmp = cmp_leading_dir_glob((char *)filename, ig->globs[i]);
        if (cmp == 0) {
            log_debug("file %s ignored because name matches glob pattern %s", filename, ig->globs[i]);
            free(temp);
            return 1;
        } else if (cmp == 2) {
            const char *subdir = ig->globs[i];
            size_t subdir_len = ag_subdir(&subdir);
            log_debug("file %s's subdirectories may match glob pattern %s; adding subdir %s to child's ignores", temp, ig->globs[i], subdir);
            ag_insert_str_sorted(&ag_dir->partial_glob_matches, &ag_dir->partial_glob_matches_len, subdir, subdir_len);
        } else {
            log_debug("pattern %s doesn't match file %s", ig->globs[i], filename);
        }
    }

    if (search_slashes) {
        for (i = 0; i < ig->partial_globs_len; i++) {
            int cmp = cmp_leading_dir_glob((char *)filename, ig->partial_globs[i]);
            if (cmp == 0) {
                log_debug("file %s ignored because name matches glob pattern %s", filename, ig->partial_globs[i]);
                free(temp);
                return 1;
            } else if (cmp == 2) {
                const char *subdir = ig->partial_globs[i];
                size_t subdir_len = ag_subdir(&subdir);
                log_debug("file %s's subdirectories may match glob pattern %s; adding subdir %s to child's ignores", temp, ig->partial_globs[i], subdir);
                if (is_fnmatch(subdir)) {
                    ag_insert_str_sorted(&ag_dir->partial_glob_matches, &ag_dir->partial_glob_matches_len, subdir, subdir_len);
                } else {
                    ag_insert_str_sorted(&ag_dir->partial_name_matches, &ag_dir->partial_name_matches_len, subdir, subdir_len);
                }
            } else {
                log_debug("pattern %s doesn't match file %s", ig->partial_globs[i], filename);
            }
        }
    }

    int rv = ackmate_dir_match(temp);
    free(temp);
    return rv;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(const char *path, struct ag_dirent *ag_dir, void *baton_) {
    const struct dirent *dir = ag_dir->dirent;
    const char *filename = dir->d_name;
/* TODO: don't call strlen on filename every time we call filename_filter() */
#ifdef HAVE_DIRENT_DNAMLEN
    size_t filename_len = dir->d_namlen;
#else
    size_t filename_len = strlen(filename);
#endif
    size_t i;
    scandir_baton_t *baton = (scandir_baton_t *)baton_;
    struct ignore_iters *iters = &baton->iters;
    const ignores *ig = baton->ig;
    const char *base_path = baton->base_path;
    const size_t base_path_len = baton->base_path_len;
    const char *path_start = path;
    int search_slashes = 1;

    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }

    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            return 0;
        }
    }

    if (!opts.follow_symlinks && is_symlink(path, dir)) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return 0;
    }

    if (is_named_pipe(path, dir)) {
        log_debug("%s ignored because it's a named pipe", path);
        return 0;
    }

    if (opts.search_all_files && !opts.path_to_agignore) {
        return 1;
    }

    for (i = 0; base_path[i] == path[i] && i < base_path_len; i++) {
        /* base_path always ends with "/\0" while path doesn't, so this is safe */
        path_start = path + i + 2;
    }
    log_debug("path_start %s filename %s", path_start, filename);

    const char *extension = strchr(filename, '.');
    if (extension) {
        if (extension[1]) {
            // The dot is not the last character, extension starts at the next one
            ++extension;
        } else {
            // No extension
            extension = NULL;
        }
    }

    while (1) {
        if (strncmp(filename, "./", 2) == 0) {
            filename++;
            filename_len--;
        }

        if (extension) {
            int match_pos = binary_search(extension, ig->extensions, 0, ig->extensions_len);
            if (match_pos >= 0) {
                log_debug("file %s ignored because name matches extension %s", filename, ig->extensions[match_pos]);
                return 0;
            }
        }

        if (path_ignore_search(ig, path_start, filename, iters, search_slashes, ag_dir)) {
            return 0;
        }

        ig = ig->parent;
        if (ig == NULL) {
            break;
        }
        if (iters->parent == NULL) {
            iters->parent = malloc(sizeof(*iters->parent));
            if (iters->parent == NULL) {
                return -1;
            }
            *iters->parent = (struct ignore_iters){0, 0, 0, 0, 0, 0, NULL};
        }
        iters = iters->parent;
        search_slashes = 0;
    }

    log_debug("%s not ignored", filename);
    return 1;
}
