#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE
};

typedef struct {
    int after;
    int before;
    int context;
    int ackmate;
    int recurse_dirs;
    int follow_symlinks;
    enum case_behavior casing;
} cli_options;

cli_options *parse_options(int argc, char **argv);

#endif
