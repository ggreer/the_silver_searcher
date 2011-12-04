#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "options.h"

static struct option longopts[] = {
    { "after", required_argument, NULL, 'A' },
    { "before", required_argument, NULL, 'B' },
    { "context", required_argument, NULL, 'C' },
    { "ackmate", no_argument, NULL, 0 },
    { "follow", no_argument, NULL, 'f' },
    { "nofollow", no_argument, NULL, 0 },
    { "ignore-case", no_argument, NULL, 0 },
    { "nosmart-case", no_argument, NULL, 0 },
    { "ackmate-dir-filter", no_argument, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

cli_options *parse_options(int argc, char **argv) {
    int ch;

    cli_options *opts = malloc(sizeof(opts));

    opts->follow_symlinks = 0;
    opts->recurse_dirs = 1;
    opts->casing = CASE_SENSITIVE_RETRY_INSENSITIVE;

    // XXX: this is nowhere near done
    while ((ch = getopt_long(argc, argv, "A:B:C:f", longopts, NULL)) != -1) {
        switch (ch) {
            case 'A':
                opts->after = atoi(optarg);
                break;
            case 'B':
                opts->before = atoi(optarg);
                break;
            case 'C':
                opts->context = atoi(optarg);
                break;
            case 'f':
                opts->follow_symlinks = 1;
                break;
            default:
                // print usage here or something
                printf("blah\n");
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    return opts;
};
