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

char **ignored_patterns = NULL;

int filename_filter(struct dirent *dir) {
    //regex = pcre_compile();
    if (strcmp(dir->d_name, ".") == 0) {
        log_debug("No %s", dir->d_name);
        return(0);
    }
    else if (strcmp(dir->d_name, "..") == 0) {
        log_debug("No %s", dir->d_name);
        return(0);
    }
    log_debug("Yes %s", dir->d_name);
    return(1);
};

int search_dir(pcre *re, const char* path, const int depth) {
    log_err("depth %i", depth);
    if(depth > 8) {
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

    //TODO: recurse dirs
    results = scandir(path, &dir_list, &filename_filter, &alphasort);
    if (results == 0)
    {
        log_debug("No results found");
        return(0);
    }
    else if (results == -1) {
        log_err("Couldn't open the directory");
        exit(1);
    }

    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        log_debug("dir name %s type %i", dir->d_name, dir->d_type);
        if (dir->d_type == DT_DIR) {
            log_msg("searching dir %s", dir->d_name);
            rv = search_dir(re, dir->d_name, depth + 1);
            continue;
        }
        fp = fopen(dir->d_name, "r");
        if (fp == NULL) {
            log_warn("Error opening file %s. Skipping...", dir->d_name);
            continue;
        }

        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            plog(LOG_LEVEL_ERR, "fseek error");
            exit(1);
        }
        f_len = ftell(fp); //TODO: behave differently if file is HUGE
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
        int offset_vector[100]; //XXXX max number of matches in a file
        int rc = 0;
        while(buf_offset < buf_len && (rc = pcre_exec(re, NULL, buf, r_len, buf_offset, 0, offset_vector, sizeof(offset_vector))) >= 0 ) {
            
            log_debug("match found. file %s offset %i", dir->d_name, offset_vector[0]);
            buf_offset = offset_vector[1];
        }

        free(buf);

        cleanup:
        fclose(fp);
        free(dir);
    }

    free(dir_list);
    return(0);
};

int main(int argc, char **argv) {
    cli_options opts;
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.recurse_dirs = 1;

//    use getopts and ilk

    char *query;
    // last argument is the query
    if (argc < 2) {
        log_err("Not enough arguments :P");
        exit(1);
    }

    query = malloc(strlen(argv[argc-1])+1);
    strcpy(query, argv[argc-1]);

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

    rv = search_dir(re, "./", 0);

    return(0);
}
