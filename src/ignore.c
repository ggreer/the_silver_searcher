#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pcre.h>
#include <dirent.h>

#include "ignore.h"


const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

char **ignore_patterns = NULL;
int ignore_patterns_len = 0;

void add_ignore_pattern(const char* pattern) {
    if(ignore_patterns == NULL) {
        ignore_patterns = malloc(sizeof(char**));
    }
    ignore_patterns = realloc(ignore_patterns, ignore_patterns_len + sizeof(char*));
    ignore_patterns[ignore_patterns_len] = strdup(pattern);
    ignore_patterns[ignore_patterns_len + 1] = NULL;
    ignore_patterns_len++;
}

void cleanup_ignore_patterns() {
    for(int i = 0; i<ignore_patterns_len; i++) {
        free(ignore_patterns[i]);
    }
    free(ignore_patterns);
}

// TODO: make this a sorted array so filtering is O(log(n)) instead of O(n)
void load_ignore_patterns(const char *ignore_filename) {
    FILE *fp = NULL;
    fp = fopen(ignore_filename, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s", ignore_filename);
        return;
    }

    char *line = NULL;
    ssize_t line_length = 0;
    size_t line_cap = 0;

    while((line_length = getline(&line, &line_cap, fp)) > 0) {
        log_debug("ignoring pattern %s", line);
        add_ignore_pattern(line);
    }
}

int filename_filter(struct dirent *dir) {
/*    if (dir->d_type != DT_REG && dir->d_type != DT_DIR) {
        log_debug("file %s ignored becaused of type", dir->d_name);
        return(0);
    }
*/
    char *filename = dir->d_name;
    for (int i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            log_debug("file %s ignored because of name", filename);
            return(0);
        }
    }

    int pcre_opts = 0;
    int rc = 0;
    const char *pcre_err = NULL;
    pcre *re = NULL;
    char *pattern = NULL;
    int pcre_err_offset;
    int buf_offset = 0;
    int offset_vector[2]; //only need to grab the first match

    for (int i = 0; ignore_patterns[i] != NULL; i++) {
        pattern = ignore_patterns[i];
        re = pcre_compile(pattern, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
        rc = pcre_exec(re, NULL, filename, strlen(filename), buf_offset, 0, offset_vector, sizeof(offset_vector));
        if (rc >= 0) {
            log_debug("file %s ignored because name matches pattern %s", dir->d_name, pattern);
            return(0);
        }
        pcre_free(re);
    }

    log_debug("Yes %s", dir->d_name);
    return(1);
}