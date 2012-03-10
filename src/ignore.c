#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "util.h"

/* TODO: built a huge-ass list of files we want to ignore by default (build cache stuff, pyc files, etc) */

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

const char *ignore_pattern_files[] = {
    ".gitignore",
    ".hgignore",
    ".agignore",
    NULL
};

/* TODO: make this a sorted array so filtering is O(log(n)) instead of O(n) */
char **ignore_patterns = NULL;
int ignore_patterns_len = 0;

const int fnmatch_flags = 0 & FNM_PATHNAME;

void add_ignore_pattern(const char* pattern) {
    ignore_patterns = realloc(ignore_patterns, (ignore_patterns_len + 1) * sizeof(char**));
    ignore_patterns[ignore_patterns_len] = strdup(pattern);
    ignore_patterns_len++;
    log_debug("added ignore pattern %s", pattern);
}

void cleanup_ignore_patterns() {
    int i;
    for(i = 0; i<ignore_patterns_len; i++) {
        free(ignore_patterns[i]);
    }
    free(ignore_patterns);
}

/* For loading git/svn/hg ignore patterns */
void load_ignore_patterns(const char *ignore_filename) {
    FILE *fp = NULL;
    fp = fopen(ignore_filename, "r");
    if (fp == NULL) {
        log_err("Skipping ignore file %s", ignore_filename);
        return;
    }

    char *line = NULL;
    ssize_t line_length = 0;
    size_t line_cap = 0;

    while ((line_length = getline(&line, &line_cap, fp)) > 0) {
        if (line_length == 0 || line[0] == '\n') {
            continue;
        }
        if (line[line_length-1] == '\n') {
            line[line_length-1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(line);
    }

    free(line);
    fclose(fp);
}

int ignorefile_filter(struct dirent *dir) {
    int i;

    if (opts.search_all_files || opts.search_unrestricted) {
        return(0);
    }

    for (i = 0; ignore_pattern_files[i] != NULL; i++) {
        if (strcmp(ignore_pattern_files[i], dir->d_name) == 0) {
            log_debug("ignore pattern matched for %s", dir->d_name);
            return(1);
        }
    }
    return(0);
}

/* this function is REALLY HOT. It gets called for every file */
int filename_filter(struct dirent *dir) {
    const char *filename = dir->d_name;
    char *pattern = NULL;
    int rc = 0;
    int i;

    if (opts.follow_symlinks == 0 && dir->d_type == DT_LNK) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return(0);
    }

    if (!opts.search_unrestricted && filename[0] == '.') {
        return(0);
    }

    if (opts.search_all_files) {
        return(1);
    }

    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            log_err("file %s ignored because of name", filename);
            return(0);
        }
    }

    for (i = 0; i<ignore_patterns_len; i++) {
        pattern = ignore_patterns[i];
        if (fnmatch(pattern, filename, fnmatch_flags) == 0) {
            log_debug("file %s ignored because name matches pattern %s", dir->d_name, pattern);
            return(0);
        }
    }

    if (opts.ackmate_dir_filter != NULL) {
        /* we just care about the match, not where the matches are */
        rc = pcre_exec(opts.ackmate_dir_filter, NULL, dir->d_name, strlen(dir->d_name), 0, 0, NULL, 0);
        if (rc >= 0) {
            log_err("file %s ignored because name ackmate dir filter pattern", dir->d_name);
            return(0);
        }
    }

    return(1);
}
