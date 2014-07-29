#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>
#include <sys/stat.h>

#include <pcre.h>

#define DEFAULT_CONTEXT_LEN 2

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
    size_t after;
    size_t before;
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
    int print_matches_only;
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
    size_t stream_line_num; /* This should totally not be in here */
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

typedef struct option option_t;

void usage(void);
void print_version(void);

void init_options(void);
void parse_options(int argc, char **argv, char **base_paths[], char **paths[]);
void cleanup_options(void);

#endif
