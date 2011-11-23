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
    int f_len, r_len;
    char *buf = NULL;

    results = scandir("./", &dir_list, &filename_filter, &alphasort);
    if (results == 0)
    {
        die("No results found");
    }
    else if (results == -1) {
        die("Couldn't open the directory");
    }

    printf("Found %i results\n", results);
    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        fp = fopen(dir->d_name, "r");
        if (!fp) {
            printf("Error opening file %s. Skipping...\n", dir->d_name);
            continue;
        }
        f_len = fseek(fp, 0, SEEK_END);
        rewind(fp);
        buf = (char*) malloc(sizeof(char) * f_len);
        r_len = fread(buf, f_len, 1, fp);
        
        free(buf);
        fclose(fp);
        printf("match file: %s\n", dir->d_name);
        free(dir);
    }

    free(dir_list);
    return(0);
}
