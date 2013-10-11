#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>
#include <sys/stat.h>

#include <pcre.h>

#define DEFAULT_CONTEXT_LEN 2
#define DEFAULT_MAX_PRINTABLE_LINE_LENGTH 2000

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SMART,
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
    int match_files;
    pcre *file_search_regex;
    pcre_extra *file_search_regex_extra;
    int color;
    char *color_line_number;
    char *color_match;
    char *color_path;
    int column;
    int context;
    int follow_symlinks;
    int invert_match;
    int literal;
    int literal_starts_wordchar;
    int literal_ends_wordchar;
    int max_matches_per_file;
    int max_search_depth;
    char *path_to_agignore;
    int print_break;
    int print_filename_only;
    int print_heading;
    int print_line_numbers;
    int print_long_lines; /* TODO: support this in print.c */
    int max_printable_line_length;
    pcre *re;
    pcre_extra *re_extra;
    int recurse_dirs;
    int search_all_files;
    int skip_vcs_ignores;
    int search_binary_files;
    int search_zip_files;
    int search_hidden_files;
    int search_stream; /* true if tail -F blah | ag */
    int stats;
    ino_t stdout_inode;
    char *query;
    int query_len;
    char *pager;
    int paths_len;
    int parallel;
    int word_regexp;
    int workers;
} cli_options;

/* global options. parse_options gives it sane values, everything else reads from it */
cli_options opts;

void init_options();
void parse_options(int argc, char **argv, char **base_paths[], char **paths[]);
void cleanup_options();

void usage();

#endif
