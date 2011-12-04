#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "options.h"

static struct option longopts[] = {
    { "after", required_argument, NULL, 'A' },
    { "before", required_argument, NULL, 'B' },
    { "context", required_argument, NULL, 'C' },
    { "ackmate", no_argument, NULL, 0 },
    { "follow", no_argument, NULL, 0 },
    { "nofollow", no_argument, NULL, 0 },
    { "ignore-case", no_argument, NULL, 0 },
    { "nosmart-case", no_argument, NULL, 0 },
    { "ackmate-dir-filter", no_argument, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

cli_options *parse_options(int argc, char **argv) {
    int ch;

    cli_options *opts = malloc(sizeof(opts));

    // XXX: this is nowhere near done
    while ((ch = getopt_long(argc, argv, "ABC:", longopts, NULL)) != -1) {
        switch (ch) {
            default:
                printf("blah");
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    return opts;
};