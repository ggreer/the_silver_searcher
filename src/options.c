#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "options.h"
#include "log.h"

void parse_options(int argc, char **argv) {
    int ch;

    opts.follow_symlinks = 0;
    opts.recurse_dirs = 1;
    opts.casing = CASE_SENSITIVE_RETRY_INSENSITIVE;
    opts.ackmate = 0;

    struct option longopts[] = {
        { "after", required_argument, NULL, 'A' },
        { "before", required_argument, NULL, 'B' },
        { "context", required_argument, NULL, 'C' },
        { "ackmate", no_argument, &(opts.ackmate), 1 },
        { "follow", no_argument, &(opts.follow_symlinks), 1 },
        { "nofollow", no_argument, &(opts.follow_symlinks), 0 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "nosmart-case", no_argument, NULL, 0 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { NULL, 0, NULL, 0 }
    };

    int opt_index = 0;

    // XXX: this is nowhere near done
    while ((ch = getopt_long(argc, argv, "A:B:C:fiv", longopts, &opt_index)) != -1) {
        switch (ch) {
            case 'A':
                opts.after = atoi(optarg);
                break;
            case 'B':
                opts.before = atoi(optarg);
                break;
            case 'C':
                opts.context = atoi(optarg);
                break;
            case 'f':
                opts.follow_symlinks = 1;
                break;
            case 'i':
                opts.casing = CASE_INSENSITIVE;
                break;
            case 'v':
                set_log_level(LOG_LEVEL_MSG);
                break;
            case 0: // Long option
                // Continue to usage if we don't recognize the option
                if (longopts[opt_index].flag != 0) {
                    break;
                }
                log_err("Unknown option %s");
            default:
                usage();
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;
}

void usage() {
    // I am a terrible person
    printf("Usage: omg I hate writing docs just look at the source :P\n");
}
