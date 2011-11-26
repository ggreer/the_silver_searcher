#include <stdarg.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE
};

typedef struct {
    int recurse_dirs;
    enum case_behavior casing;
} cli_options;

int filename_filter(struct dirent *dir) {
    //regex = pcre_compile();
    plog(LOG_LEVEL_DEBUG, "File %s", dir->d_name);
    return(1);
};

int main(int argc, char **argv) {
    cli_options opts;
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.recurse_dirs = 1;

//    use getopts and ilk

    char *query;
    // last argument is the query
    if (argc < 2) {
        plog(LOG_LEVEL_ERR, "Not enough arguments :P\n");
        exit(1);
    }
    query = malloc(strlen(argv[argc-1])+1);
    strcpy(query, argv[argc-1]);

    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    FILE *fp = NULL;
    int f_len;
    size_t r_len;
    char *buf = NULL;
    int rv = 0;

    //TODO: recurse dirs
    results = scandir("./", &dir_list, &filename_filter, &alphasort);
    if (results == 0)
    {
        plog(LOG_LEVEL_ERR, "No results found\n");
        exit(1);
    }
    else if (results == -1) {
        plog(LOG_LEVEL_ERR, "Couldn't open the directory\n");
        exit(1);
    }

    int pcre_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;
    re = pcre_compile(query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (re == NULL) {
        plog(LOG_LEVEL_ERR, "pcre_compile failed\n");
        exit(1);
    }

    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        plog(LOG_LEVEL_DEBUG, "dir name %s type %i\n", dir->d_name, dir->d_type);
        fp = fopen(dir->d_name, "r");
        if (fp == NULL) {
            plog(LOG_LEVEL_WARN, "Error opening file %s. Skipping...\n", dir->d_name);
            continue;
        }

        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            plog(LOG_LEVEL_ERR, "fseek error");
            exit(1);
        }
        f_len = ftell(fp); //TODO: behave differently if file is HUGE
        if (f_len == 0) {
            plog(LOG_LEVEL_DEBUG, "file is empty\n");
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
            
            printf("match found. file %s offset %i\n", dir->d_name, offset_vector[0]);
            buf_offset = offset_vector[1];
        }

        free(buf);

        cleanup:
        fclose(fp);
        free(dir);
    }

    free(dir_list);
    return(0);
}
