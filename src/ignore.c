#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "util.h"

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
    ".hgignore",
    ".svn",
    NULL
};

const int fnmatch_flags = 0 & FNM_PATHNAME;

ignores *init_ignore(ignores *parent) {
    ignores *ig = ag_malloc(sizeof(ignores));
    ig->names = NULL;
    ig->names_len = 0;
    ig->regexes = NULL;
    ig->regexes_len = 0;
    ig->parent = parent;
    return ig;
}

void cleanup_ignore(ignores *ig) {
    size_t i;

    if (ig) {
        if (ig->regexes) {
            for (i = 0; i < ig->regexes_len; i++) {
                free(ig->regexes[i]);
            }
            free(ig->regexes);
        }
        if (ig->names) {
            for (i = 0; i < ig->names_len; i++) {
                free(ig->names[i]);
            }
            free(ig->names);
        }
        free(ig);
    }
}

void add_ignore_pattern(ignores *ig, const char* pattern) {
    int i;

    /* Strip off the leading ./ so that matches are more likely. */
    if (strncmp(pattern, "./", 2) == 0) {
        pattern += 2;
    }

    if (is_fnmatch(pattern)) {
        ig->regexes_len++;
        ig->regexes = ag_realloc(ig->regexes, ig->regexes_len * sizeof(char*));
        ig->regexes[ig->regexes_len - 1] = ag_strdup(pattern);
        log_debug("added regex ignore pattern %s", pattern);
    } else {
        /* a balanced binary tree is best for performance, but I'm lazy */
        ig->names_len++;
        ig->names = ag_realloc(ig->names, ig->names_len * sizeof(char*));
        for (i = ig->names_len - 1; i > 0; i--) {
            if (strcmp(pattern, ig->names[i-1]) > 0) {
                break;
            }
            ig->names[i] = ig->names[i-1];
        }
        ig->names[i] = ag_strdup(pattern);
        log_debug("added literal ignore pattern %s", pattern);
    }
}

/* For loading git/svn/hg ignore patterns */
void load_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s", path);
        return;
    }

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len-1] == '\n') {
            line[line_len-1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(ig, line);
    }

    free(line);
    fclose(fp);
}

void load_svn_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    char *dir_prop_base;
    asprintf(&dir_prop_base, "%s/%s", path, SVN_DIR_PROP_BASE);

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
        key = ag_realloc(key, (key_len + 1) * sizeof(char));
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
    while (*patterns != '\0' && patterns < (entry + bytes_read)) {
        for (line_len = 0; line_len < strlen(patterns); line_len++) {
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
    }
    free(entry);
    cleanup:;
    free(dir_prop_base);
    free(key);
    fclose(fp);
}

int ackmate_dir_match(const char* dir_name) {
    int rc = 0;

    if (opts.ackmate_dir_filter != NULL) {
        /* we just care about the match, not where the matches are */
        rc = pcre_exec(opts.ackmate_dir_filter, NULL, dir_name, strlen(dir_name), 0, 0, NULL, 0);
        if (rc >= 0) {
            log_debug("file %s ignored because name matches ackmate dir filter pattern", dir_name);
            return 1;
        }
    }

    return 0;
}

int filename_ignore_search(const ignores *ig, const char *filename) {
    size_t i;
    int match_pos;

    match_pos = binary_search(filename, ig->names, 0, ig->names_len);
    if (match_pos >= 0) {
        log_debug("file %s ignored because name matches static pattern %s", filename, ig->names[match_pos]);
        return 1;
    }

    if (ackmate_dir_match(filename)) {
        log_debug("file %s ignored because name matches ackmate regex", filename);
        return 1;
    }

    for (i = 0; i < ig->regexes_len; i++) {
        if (fnmatch(ig->regexes[i], filename, fnmatch_flags) == 0) {
            log_debug("file %s ignored because name matches regex pattern %s", filename, ig->regexes[i]);
            return 1;
        }
    }
    return 0;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(const char *path, const struct dirent *dir, void *baton) {
    const char *filename = dir->d_name;
    size_t i;
    ignores *ig = (ignores*) baton;

    if (!opts.follow_symlinks && is_symlink(path, dir)) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return 0;
    }

    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            return 0;
        }
    }

    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }
    if (opts.search_all_files && !opts.path_to_agignore) {
        return 1;
    }
    if (filename_ignore_search(ig, filename)) {
        return 0;
    }

    if (is_directory(path, dir) && filename[strlen(filename) - 1] != '/') {
        char *temp;
        asprintf(&temp, "%s/", filename);
        int rv = filename_ignore_search(ig, temp);
        free(temp);
        if (rv) {
            return 0;
        }
    }

    if (ig->parent != NULL) {
        return filename_filter(path, dir, (void *)(ig->parent));
    }

    return 1;
}
