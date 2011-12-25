#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>
#include <pcre.h>

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE // for future use
};

typedef struct {
    int ackmate;
    pcre *ackmate_dir_filter;
    int after;
    int before;
    enum case_behavior casing;
    int color;
    int context;
    int follow_symlinks;
    int literal;
    int print_heading;
    int print_break;
    int print_filename_only;
    int recurse_dirs;
} cli_options;

// global options. parse_options gives it sane values, everything else reads from it
cli_options opts;

void init_options();
void parse_options(int argc, char **argv);
void cleanup_options();

void usage();

#endif
