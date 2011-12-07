#include <dirent.h>
#include <pcre.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/types.h>

#include "ignore.h"
#include "log.h"
#include "options.h"

const int MAX_SEARCH_DEPTH = 100;

/*
  Example output of ackmate's ack:
  ./ackmate_ack --ackmate --literal --context --nofollow --ignore-case --match "test" ../
  :../Resources/ackmate_ack
  756:
  757:That's why ack's behavior of not searching things it doesn't recognize
  758;18 4:is one of its greatest strengths: the speed you get from only
  759:searching the things that you want to be looking at.
  760:
  --
  813:issues list at Github: L<http://github.com/petdance/ack/issues>
  814:
  815;45 4:Patches are always welcome, but patches with tests get the most
  816:attention.
  817:
  
  simpler output:
  :../Resources/ackmate_ack
  758;18 4:is one of its greatest strengths: the speed you get from only
  815;45 4:Patches are always welcome, but patches with tests get the most
 */
// TODO: this function is hacks upon hacks. rewrite it once behavior is correct
void print_match(const char* path, const char* buf, char* match_start, char* match_end, int first_match) {
    char *match_bol = match_start;
    char *match_eol = NULL;
    int line = 1;
    int column = 0;
    // find start of line
    while (match_bol > buf && *match_bol != '\n') {
        match_bol--;
        column++;
    }
    if (*match_bol == '\n') {
        column--;
    }

    // find line number. XXXXX this is extremely ineffecient for files with multiple matches
    for (char *i = match_bol; i > buf; i--) {
        if (*i == '\n') {
            line++;
        }
    }
    if (first_match) {
        printf(":%s\n", path); //print the path only if this is the first match in this file
    }

    // print context before match line
    for (int i = 0; i < opts.before && match_bol > buf; i++) {
        while (match_bol > buf && *match_bol != '\n') {
            match_bol--;
        }
    }
    // XXXX this will screw up if the previous line is empty or something.
    // basically this function needs to be more stateful
    if (*match_bol == '\n') {
        match_bol++;
    }
    // MAKE IT RED
//    printf("\e[31m%s\e[0m:", path);
    printf("%i;%i %i:", line, column, (int)(match_end - match_start));
    // print line start to start of match
    for (char *j = match_bol; j<match_start; j++) {
        putchar(*j);
    }
    // print match
    for (char *j = match_start; j<match_end; j++) {
        putchar(*j);
    }
    // print end of match to end of line
    match_eol = match_end;
    for (char *j = match_end; *j != '\n'; j++) {
        putchar(*j);
        match_eol = j;
    }
    putchar('\n');

    // print context after match line
    for (int i = 0; i < opts.after; i++) {
        for (char *j = match_eol; *j != '\n'; j++) {
            putchar(*j);
        }
        putchar('\n');
        line++;
        printf("%i: ", line);
    }
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
            free(dir_full_path);
        }
    }
    free(dir_list);

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

    for (int i=0; i<results; i++) {
        dir = dir_list[i];
        // XXX: this is copy-pasted from about 30 lines above
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
            log_err("Error opening file %s. Skipping...", dir_full_path);
            goto cleanup;
            continue;
        }

        rv = fseek(fp, 0, SEEK_END);
        if (rv != 0) {
            log_err("Error fseek()ing file %s. Skipping...", dir_full_path);
            goto cleanup;
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
        int first_match = 1;
        // In my profiling, most of the execution time is spent in this pcre_exec
        while(buf_offset < buf_len && (rc = pcre_exec(re, NULL, buf, r_len, buf_offset, 0, offset_vector, sizeof(offset_vector))) >= 0 ) {
            log_debug("Match found. File %s, offset %i bytes.", dir_full_path, offset_vector[0]);
            match_start = buf + offset_vector[0];
            match_end = buf + offset_vector[1];
            buf_offset = offset_vector[1];
            print_match(dir_full_path, buf, match_start, match_end, first_match);
            first_match = 0;
        }

        free(buf);

        cleanup:
        if (fp != NULL) {
            fclose(fp);
        }
        free(dir);
        free(dir_full_path);
    }

    free(dir_list);
    return(0);
}

int main(int argc, char **argv) {
    set_log_level(LOG_LEVEL_ERR);
//    set_log_level(LOG_LEVEL_DEBUG);

///*
    for(int i = 0; i < argc; i++) {
        fprintf(stderr, "%s ", argv[i]);
    }
//*/
    char *query;
    char *path;
    int pcre_opts = 0;
    int rv = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;

    parse_options(argc, argv);

    query = malloc(strlen(argv[argc-2])+1);
    strcpy(query, argv[argc-2]);
    path = malloc(strlen(argv[argc-1])+1);
    strcpy(path, argv[argc-1]);

    if (opts.casing == CASE_INSENSITIVE) {
        pcre_opts |= PCRE_CASELESS;
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
