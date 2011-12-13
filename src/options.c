#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "options.h"
#include "log.h"

void parse_options(int argc, char **argv) {
    int ch;

    opts.follow_symlinks = 0;
    opts.recurse_dirs = 1;
    opts.casing = CASE_SENSITIVE;
    opts.ackmate = 0;
    opts.after = 0;
    opts.before = 0;
    opts.context = 0;

    int blah = 0;

    // XXXX: actually obey these options instead of disregarding them
    struct option longopts[] = {
        { "after", required_argument, NULL, 'A' },
        { "before", required_argument, NULL, 'B' },
        { "context", optional_argument, &(opts.context), 2 },
        { "ackmate", no_argument, &(opts.ackmate), 1 },
        { "follow", no_argument, &(opts.follow_symlinks), 1 },
        { "nofollow", no_argument, &(opts.follow_symlinks), 0 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "nosmart-case", no_argument, &blah, 0 },
        { "match", no_argument, &blah, 0 },
        { "literal", no_argument, &blah, 0 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { NULL, 0, NULL, 0 }
    };

    int opt_index = 0;

    if (argc < 2) {
        usage();
        exit(1);
    }

    // XXX: this is nowhere near done
    // TODO: check for insane params. nobody is going to want 5000000 lines of context, for example
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
                log_err("option %s does not take a value", longopts[opt_index].name);
            default:
                usage();
                exit(1);
        }
    }
    if (opts.context > 0) {
        opts.before = opts.context;
        opts.after = opts.context;
    }

    argc -= optind;
    argv += optind;
}

void usage() {
    // I am a terrible person
    printf("Usage: omg I hate writing docs just look at the source :P\n");
}
