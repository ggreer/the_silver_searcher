#include <stdarg.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <dirent.h>

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE
};

typedef struct {
    int recurse_dirs;
    enum case_behavior casing;
} cli_options;

int filename_filter(struct dirent *dir_list) {
    return(1);
}

int main(int argc, char **argv) {
//    cli_options opts;
    struct dirent **dir_list;
    struct dirent *dir;
    int results = 0;

    results = scandir("./", &dir_list, &filename_filter, &alphasort);
    if (results > 0)
    {
        printf("Found %i results\n", results);
        for (int i=0; i<results; i++) {
            dir = dir_list[i];
            printf("%s\n", dir->d_name);
        }
    }
    else if (results == -1) {
        perror("Couldn't open the directory");
    }

    return(0);
}
