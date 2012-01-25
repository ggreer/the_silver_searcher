#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>
#include <pcre.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE /* for future use */
};

typedef struct {
    int ackmate;
    pcre *ackmate_dir_filter;
    pcre_extra *ackmate_dir_filter_extra;
    int after;
    int before;
    enum case_behavior casing;
    const char *file_search_string;
    pcre *file_search_regex;
    pcre_extra *file_search_regex_extra;
    int color;
    int column;
    int context;
    int follow_symlinks;
    int invert_match;
    int literal;
    int print_break;
    int print_filename_only;
    int print_heading;
    int print_long_lines; /* TODO: support this in print.c */
    int print_matches;
    int recurse_dirs;
    int search_binary_files;
    int search_stdin; /* true if tail -F blah | ag */
    int stats;
    char *query;
    int query_len;
} cli_options;

/* global options. parse_options gives it sane values, everything else reads from it */
cli_options opts;

void init_options();
void parse_options(int argc, char **argv, char **query, char **path);
void cleanup_options();

void usage();

#endif
