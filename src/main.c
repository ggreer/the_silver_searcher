#include <stdarg.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "options.h"

cli_options opts;

const int MAX_SEARCH_DEPTH = 100;

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

char *ignore_patterns[] = { NULL };

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

void print_match(const char* path, const char* buf, char* match_start, char* match_end) {
    char *match_bol = match_start;
    while (match_bol > buf && *match_bol != '\n') {
        match_bol--;
    }
    if (*match_bol == '\n') {
        match_bol++;
    }
    // MAKE IT RED
    printf("\e[31m%s\e[0m:", path);
    // print line start to start of match
    for (char *j = match_bol; j<match_start; j++) {
        putchar(*j);
    }
    // print match
    for (char *j = match_start; j<match_end; j++) {
        putchar(*j);
    }
    // print end of match to end of line
    for (char *j = match_end; *j != '\n'; j++) {
        putchar(*j);
    }
    putchar('\n');
}

//TODO: append matches to some data structure instead of just printing them out
// then there can be sweet summaries of matches/files scanned/time/etc
int search_dir(pcre *re, const char* path, const int depth) {
    //TODO: don't just die. also make max depth configurable
    if(depth > MAX_SEARCH_DEPTH) {
        log_err("Search depth greater than %i, giving up.", depth);
        exit(1);
    }
    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    FILE *fp = NULL;
    int f_len;
    size_t r_len;
    char *buf = NULL;
    int rv = 0;

    results = scandir(path, &dir_list, &filename_filter, &alphasort);
    if (results == 0)
    {
        log_debug("No results found");
        free(dir_list);
        return(0);
    }
    else if (results == -1) {
        log_err("Error opening directory %s", path);
        return(0);
    }

    char *dir_full_path = NULL;
    size_t path_length = 0;

    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        path_length = (size_t)(strlen(path) + strlen(dir->d_name) + 2); // 2 for slash and null char
        dir_full_path = malloc(path_length);
        dir_full_path = strncpy(dir_full_path, path, path_length);
        dir_full_path = strncat(dir_full_path, "/", path_length);
        dir_full_path = strncat(dir_full_path, dir->d_name, path_length);

        log_debug("dir %s type %i", dir_full_path, dir->d_type);
        //TODO: scan files in current dir before going deeper
        if (dir->d_type == DT_DIR && opts.recurse_dirs) {
            log_debug("searching dir %s", dir_full_path);
            rv = search_dir(re, dir_full_path, depth + 1);
            goto cleanup;
            continue;
        }
        fp = fopen(dir_full_path, "r");
        if (fp == NULL) {
            log_warn("Error opening file %s. Skipping...", dir_full_path);
            goto cleanup;
            continue;
        }

        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            log_err("fseek error");
            exit(1);
        }
        f_len = ftell(fp); //TODO: behave differently if file is HUGE. on 32 bit, anything > 2GB will screw up this program
        if (f_len == 0) {
            log_debug("file is empty. skipping");
            goto cleanup;
        }

        rewind(fp);
        buf = (char*) malloc(sizeof(char) * f_len + 1);
        r_len = fread(buf, 1, f_len, fp);
        buf[r_len] = '\0';
        int buf_len = (int)r_len;

        int buf_offset = 0;
        int offset_vector[100]; //XXXX max number of matches in a file / 2
        int rc = 0;
        char *match_start = NULL;
        char *match_end = NULL;
        while(buf_offset < buf_len && (rc = pcre_exec(re, NULL, buf, r_len, buf_offset, 0, offset_vector, sizeof(offset_vector))) >= 0 ) {
            log_debug("match found. file %s offset %i", dir_full_path, offset_vector[0]);
            match_start = buf + offset_vector[0];
            match_end = buf + offset_vector[1];
            print_match(dir_full_path, buf, match_start, match_end);
            buf_offset = offset_vector[1];
        }

        free(buf);

        cleanup:
        fclose(fp); // sometimes fp is null. the manpage says this should segfault but... it works so whatever
        free(dir);
        free(dir_full_path);
    }

    free(dir_list); // XXX I'm pretty sure this isn't right. we need to iterate and free the entries
    return(0);
}

void add_ignore_pattern(const char* pattern) {
    
}

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
        add_ignore_pattern(line);
    }
}

int main(int argc, char **argv) {
    set_log_level(LOG_LEVEL_ERR);
//    set_log_level(LOG_LEVEL_DEBUG);
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.recurse_dirs = 1;

    //TODO: use getopts and ilk
    char *query;
    char *path;
    // last argument is the query
    if (argc < 3) {
        log_err("Not enough arguments :P");
        exit(1);
    }

    load_ignore_patterns(".gitignore");

    query = malloc(strlen(argv[1])+1);
    strcpy(query, argv[1]);
    path = malloc(strlen(argv[2])+1);
    strcpy(path, argv[2]);

    int rv = 0;
    int pcre_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;
    re = pcre_compile(query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (re == NULL) {
        log_err("pcre_compile failed");
        exit(1);
    }

    rv = search_dir(re, path, 0);

    pcre_free(re);
    free(query);
    free(path);

    return(0);
}
