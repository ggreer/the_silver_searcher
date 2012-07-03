#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

const char *ignore_pattern_files[] = {
    ".agignore",
    ".gitignore",
    ".hgignore",
    NULL
};

/* For patterns that need fnmatch */
char **ignore_patterns = NULL;
int ignore_patterns_len = 0;

/* For patterns with no regex stuff in them. Sorted for fast matching. */
char **ignore_names = NULL;
int ignore_names_len = 0;

const int fnmatch_flags = 0 & FNM_PATHNAME;

void add_ignore_pattern(const char* pattern) {
    int i;

    if (is_fnmatch(pattern)) {
        ignore_patterns_len++;
        ignore_patterns = realloc(ignore_patterns, (ignore_patterns_len) * sizeof(char*));
        ignore_patterns[ignore_patterns_len - 1] = strdup(pattern);
        log_debug("added regex ignore pattern %s", pattern);
    }
    else {
        /* a balanced binary tree is best for performance, but I'm lazy */
        ignore_names_len++;
        ignore_names = realloc(ignore_names, ignore_names_len * sizeof(char*));
        for (i = ignore_names_len-1; i > 0; i--) {
            if (strcmp(pattern, ignore_names[i-1]) > 0) {
                break;
            }
            ignore_names[i] = ignore_names[i-1];
        }
        ignore_names[i] = strdup(pattern);
        log_debug("added literal ignore pattern %s", pattern);
    }
}

void cleanup_ignore_patterns() {
    int i;
    for (i = 0; i<ignore_patterns_len; i++) {
        free(ignore_patterns[i]);
    }
    free(ignore_patterns);

    for (i = 0; i<ignore_names_len; i++) {
        free(ignore_names[i]);
    }
    free(ignore_names);
}

void load_svn_ignore_patterns(const char *path, const int path_len) {
    FILE *fp = NULL;
    char *dir_prop_base = malloc(path_len + strlen(SVN_DIR_PROP_BASE) + 1);
    strlcpy(dir_prop_base, path, path_len + 1);
    strlcat(dir_prop_base, SVN_DIR_PROP_BASE, path_len + strlen(SVN_DIR_PROP_BASE) + 1);

    fp = fopen(dir_prop_base, "r");
    if (fp == NULL) {
        log_debug("Skipping svn ignore file %s", dir_prop_base);
        return;
    }

    char *entry = NULL;
    size_t entry_len = 0;
    char *key = NULL;
    size_t key_len = 0;
    size_t bytes_read = 0;
    char *entry_line = NULL;
    size_t line_len;

    while (fscanf(fp, "K %zu", &key_len) == 1) {
        key = malloc(key_len + 1);
        log_err("key len: %u", key_len);
        bytes_read = fread(key, key_len, 1, fp);
        key[bytes_read] = '\0';
        fscanf(fp, "V %zu", &entry_len); /* TODO: make sure fscanf worked */
        if (strncmp(SVN_PROP_IGNORE, key, bytes_read) != 0) {
            log_err("key is %s, not %s. continuing", key, SVN_PROP_IGNORE);
            free(key);
            /* Not the key we care about. fseek and repeat */
            fseek(fp, entry_len, SEEK_CUR);
            continue;
        }
        free(key);
        /* Aww yeah. Time to ignore stuff */
        entry = malloc(entry_len + 1);
        bytes_read = fread(entry, entry_len, 1, fp);
        entry[bytes_read] = '\0';
        log_err("entry: %s", entry);
        char *patterns = entry;
        while (patterns != NULL) {
            for (line_len = 0; line_len < strlen(patterns); line_len++) {
                if (patterns[line_len] == '\n') {
                    break;
                }
            }
            entry_line = malloc((size_t)line_len + 1);
            strlcpy(entry_line, entry, line_len);
            log_err("adding ignore pattern %s", entry_line);
            exit(0);
            add_ignore_pattern(entry_line);
            free(entry_line);
            if (patterns > (entry + entry_len)) {
                log_err("WE SHOULD NEVER GET HERE!");
                exit(1);
            }
        }
        free(entry);
    }
    fclose(fp);
}

/* For loading git/svn/hg ignore patterns */
void load_ignore_patterns(const char *ignore_filename) {
    FILE *fp = NULL;
    fp = fopen(ignore_filename, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s", ignore_filename);
        return;
    }

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n') {
            continue;
        }
        if (line[line_len-1] == '\n') {
            line[line_len-1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(line);
    }

    free(line);
    fclose(fp);
}

int ignorefile_filter(struct dirent *dir) {
    int i;

    if (opts.search_all_files || opts.search_unrestricted) {
        return 0;
    }

    /* This isn't as bad as it looks. ignore_pattern_files only has 3 elements */
    for (i = 0; ignore_pattern_files[i] != NULL; i++) {
        if (strcmp(ignore_pattern_files[i], dir->d_name) == 0) {
            log_debug("ignore pattern matched for %s", dir->d_name);
            return 1;
        }
    }

    if (strcmp(SVN_DIR, dir->d_name) == 0) {
        log_debug("svn ignore pattern matched for %s", dir->d_name);
        load_svn_ignore_patterns(dir->d_name, strlen(dir->d_name));
        return 1;
    }

    return 0;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(struct dirent *dir) {
    const char *filename = dir->d_name;
    int match_pos;
    char *pattern = NULL;
    int rc = 0;
    int i;

    if (!opts.follow_symlinks && dir->d_type == DT_LNK) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return 0;
    }

    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            log_debug("file %s ignored because it was in evil_hardcoded_ignore_files", filename);
            return 0;
        }
    }

    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }

    if (opts.search_all_files) {
        return 1;
    }

    match_pos = binary_search(dir->d_name, ignore_names, 0, ignore_names_len);
    if (match_pos >= 0) {
        log_debug("file %s ignored because name matches static pattern %s", dir->d_name, ignore_names[match_pos]);
        return 0;
    }

    for (i = 0; i < ignore_patterns_len; i++) {
        pattern = ignore_patterns[i];
        if (fnmatch(pattern, filename, fnmatch_flags) == 0) {
            log_debug("file %s ignored because name matches regex pattern %s", dir->d_name, pattern);
            return 0;
        }
    }

    if (opts.ackmate_dir_filter != NULL) {
        /* we just care about the match, not where the matches are */
        rc = pcre_exec(opts.ackmate_dir_filter, NULL, dir->d_name, strlen(dir->d_name), 0, 0, NULL, 0);
        if (rc >= 0) {
            log_debug("file %s ignored because name matches ackmate dir filter pattern", dir->d_name);
            return 0;
        }
    }

    return 1;
}
