#include <stdarg.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>

enum log_level {
    LOG_LEVEL_DEBUG = 10,
    LOG_LEVEL_MSG   = 20,
    LOG_LEVEL_WARN  = 30,
    LOG_LEVEL_ERR   = 40
};

const enum log_level log_threshold = LOG_LEVEL_DEBUG;

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
//    regex = pcre_compile();
    return(1);
}

void die(const char *message) {
    perror(message);
    exit(1);
}

void log(const int level, const char *fmt, ...) {
    if (level <= log_threshold) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
};

int main(int argc, char **argv) {
    cli_options opts;
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.recurse_dirs = 1;
    char *query;
    // last argument is the query
    if (argc < 2) {
        die("Not enough arguments :P\n");
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
        die("No results found");
    }
    else if (results == -1) {
        die("Couldn't open the directory");
    }

    int pcre_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;
    re = pcre_compile(query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    if (re == NULL) {
        die("pcre_compile failed");
    }

    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        fp = fopen(dir->d_name, "r");
        if (fp == NULL) {
            printf("Error opening file %s. Skipping...\n", dir->d_name);
            continue;
        }

        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            die("fseek error");
        }
        f_len = ftell(fp); //TODO: behave differently if file is HUGE
        if (f_len == 0) {
            printf("file is empty\n");
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
