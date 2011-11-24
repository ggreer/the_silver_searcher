#include <stdarg.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>

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
    printf("%s\n", dir->d_name);
    return(1);
}

void die(char *message) {
    perror(message);
    exit(1);
}

int main(int argc, char **argv) {
    cli_options opts;
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.recurse_dirs = 1;
    char *query;
    printf("%i", argc);
    // last argument is the query
    if (argc < 2) {
        die("Not enough arguments :P\n");
    }
    query = malloc(strlen(argv[argc-1])+1);
    strcpy(query, argv[argc-1]);
    printf("query is %s\n", query);

    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    FILE *fp = NULL;
    int f_len;
    size_t r_len;
    char *buf = NULL;
    int rv = 0;

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

    printf("Found %i results\n", results);
    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        fp = fopen(dir->d_name, "r");
        if (fp == NULL) {
            printf("Error opening file %s. Skipping...\n", dir->d_name);
            continue;
        }
        printf("opened %s\n", dir->d_name);
        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            die("fseek error");
        }
        f_len = ftell(fp);
        printf("f_len: %i", f_len);
        if (f_len == 0) {
            printf("file is empty\n");
            goto cleanup;
        }

        rewind(fp);
        buf = (char*) malloc(sizeof(char) * f_len + 1);
        r_len = fread(buf, 1, f_len, fp);
        buf[r_len] = '\0';

        printf("file length: %u\n", (unsigned int)r_len);
        //printf("file %s\n", buf);

        int buf_offset = 0;
        int offset_vector[100]; //XXXX max number of matches in a file
        int rc = 0;
        rc = pcre_exec(re, NULL, buf, r_len, buf_offset, 0, offset_vector, sizeof(offset_vector));
        

        free(buf);

        cleanup:
        fclose(fp);
        free(dir);
    }

    free(dir_list);
    return(0);
}
