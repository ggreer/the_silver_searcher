#include <dirent.h>
#include <limits.h>
#include <pcre.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"

const int MAX_SEARCH_DEPTH = 100;
const int MAX_MATCHES_PER_FILE = 100;

int is_binary(const char* buf, int buf_len) {
    for (int i = 0; i < buf_len && i < 1024; i++) {
        if (buf[i] == '\0') {
            return(1);
        }
    }

    return(0);
}

//TODO: append matches to some data structure instead of just printing them out
// then there can be sweet summaries of matches/files scanned/time/etc
int search_dir(pcre *re, const char* path, const int depth) {
    //TODO: don't just die. also make max depth configurable
    if (depth > MAX_SEARCH_DEPTH) {
        log_err("Search depth greater than %i, giving up.", depth);
        exit(1);
    }
    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    FILE *fp = NULL;
    off_t f_len = 0;
    size_t r_len = 0;
    char *buf = NULL;
    int rv = 0;
    char *dir_full_path = NULL;
    size_t path_length = 0;

    results = scandir(path, &dir_list, &ignorefile_filter, &alphasort);
    if (results > 0) {
        for (int i = 0; i < results; i++) {
            dir = dir_list[i];
            path_length = (size_t)(strlen(path) + strlen(dir->d_name) + 2); // 2 for slash and null char
            dir_full_path = malloc(path_length);
            dir_full_path = strncpy(dir_full_path, path, path_length);
            dir_full_path = strncat(dir_full_path, "/", path_length);
            dir_full_path = strncat(dir_full_path, dir->d_name, path_length);
            load_ignore_patterns(dir_full_path);
            free(dir);
            dir = NULL;
            free(dir_full_path);
            dir_full_path = NULL;
        }
    }
    free(dir_list);
    dir_list = NULL;

    results = scandir(path, &dir_list, &filename_filter, &alphasort);
    if (results == 0)
    {
        log_debug("No results found in directory %s", path);
        free(dir_list);
        dir_list = NULL;
        return(0);
    }
    else if (results == -1) {
        log_err("Error opening directory %s", path);
        return(0);
    }

    match matches[MAX_MATCHES_PER_FILE];
    int matches_len = 0;
    int buf_len = 0;
    int buf_offset = 0;
    int offset_vector[MAX_MATCHES_PER_FILE * 3]; //XXXX
    int rc = 0;
    struct stat statbuf;
    int binary = 0;
    int max_matches = 0;

    for (int i=0; i<results; i++) {
        matches_len = 0;
        buf_offset = 0;
        binary = 0;
        dir = dir_list[i];
        // XXX: this is copy-pasted from about 30 lines above
        path_length = (size_t)(strlen(path) + strlen(dir->d_name) + 2); // 2 for slash and null char
        dir_full_path = malloc(path_length);
        dir_full_path = strncpy(dir_full_path, path, path_length);
        dir_full_path = strncat(dir_full_path, "/", path_length);
        dir_full_path = strncat(dir_full_path, dir->d_name, path_length);

        log_debug("dir %s type %i", dir_full_path, dir->d_type);
        //TODO: scan files in current dir before going deeper
        if (dir->d_type == DT_DIR) {
            if (opts.recurse_dirs) {
                log_debug("Searching dir %s", dir_full_path);
                rv = search_dir(re, dir_full_path, depth + 1);
            }
            goto cleanup;
        }

        fp = fopen(dir_full_path, "r");
        if (fp == NULL) {
            log_err("Error opening file %s. Skipping...", dir_full_path);
            goto cleanup;
        }

        rv = fstat(fileno(fp), &statbuf);
        if (rv != 0) {
            log_err("Error fstat()ing file %s. Skipping...", dir_full_path);
            goto cleanup;
        }

        f_len = statbuf.st_size;

        if (f_len == 0) {
            log_debug("File %s is empty, skipping.", dir_full_path);
            goto cleanup;
        }
        else if (f_len > 1024 * 1024 * 1024) { // 1 GB
            log_err("File %s is too big. Skipping...", dir_full_path);
            goto cleanup;
        }

        rewind(fp);
        buf = (char*) malloc(sizeof(char) * f_len + 1);
        r_len = fread(buf, 1, f_len, fp);
        buf[r_len] = '\0';
        buf_len = (int)r_len;

        if (is_binary(buf, buf_len)) {
            binary = 1;
            max_matches = 1;
        }
        else {
            max_matches = MAX_MATCHES_PER_FILE;
        }

        // In my profiling, most of the execution time is spent in this pcre_exec
        while (buf_offset < buf_len &&
             (rc = pcre_exec(re, NULL, buf, buf_len, buf_offset, 0, offset_vector, max_matches)) >= 0) {
            log_debug("Match found. File %s, offset %i bytes.", dir_full_path, offset_vector[0]);
            buf_offset = offset_vector[1];
            matches[matches_len].start = offset_vector[0];
            matches[matches_len].end = offset_vector[1];
            matches_len++;
            // Don't segfault. TODO: realloc this array
            if (matches_len >= MAX_MATCHES_PER_FILE) {
                log_err("Too many matches in %s. Skipping the rest of this file.");
                break;
            }
        }

        if (rc == -1) {
            log_debug("No match in %s", dir_full_path);
        }

        if (matches_len > 0) {
            if (binary) {
                printf("Binary file %s matches.\n", dir_full_path);
            }
            else {
                if (opts.context > 0) {
                    print_file_matches_with_context(dir_full_path, buf, buf_len, matches, matches_len);
                }
                else {
                    log_debug("calling print_file_matches(%s, buf, %i, matches, %i)", dir_full_path, buf_len, matches_len);
                    print_file_matches(dir_full_path, buf, buf_len, matches, matches_len);
                }
            }
        }

        free(buf);
        buf = NULL;

        cleanup:
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        free(dir);
        dir = NULL;
        free(dir_full_path);
        dir_full_path = NULL;
    }

    free(dir_list);
    dir_list = NULL;
    return(0);
}

int main(int argc, char **argv) {
    set_log_level(LOG_LEVEL_WARN);
//    set_log_level(LOG_LEVEL_DEBUG);

    // TODO: For debugging ackmate. Remove this eventually
///*
    for (int i = 0; i < argc; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
//*/
    char *query;
    char *path;
    int path_len = 0;
    int pcre_opts = 0;
    int rv = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;

    parse_options(argc, argv);

    query = malloc(strlen(argv[argc - 2]) + 1);
    strcpy(query, argv[argc - 2]);

    path_len = strlen(argv[argc - 1]);
    path = malloc(path_len + 1);
    strcpy(path, argv[argc - 1]);
    // kill trailing slash
    if (path_len > 0 && path[path_len-1] == '/') {
        path[path_len-1] = '\0';
    }

    if (opts.casing == CASE_INSENSITIVE) {
        pcre_opts = pcre_opts | PCRE_CASELESS;
    }

    re = pcre_compile(query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (re == NULL) {
        log_err("pcre_compile failed at position %i. Error: %s", pcre_err_offset, pcre_err);
        exit(1);
    }

    rv = search_dir(re, path, 0);

    pcre_free(re);
    free(query);
    free(path);
    cleanup_ignore_patterns();

    return(0);
}
