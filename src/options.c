#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xlocale.h>

#include "options.h"
#include "log.h"
#include "version.h"

// TODO: printf()ing this is not going to scale
void usage() {
    printf("Usage: ag [OPTIONS] PATTERN PATH\n");
    printf("\n");
    printf("Recursively search for PATTERN in PATH.\n");
    printf("Like grep or ack, but faster.\n");
    printf("\n");
    printf("Example: ag -i foo /bar/\n");
    printf("\n");
    printf("Search options:\n");
    printf("\n");
    printf("  -i, --ignore-case\n");
    printf("  --literal\n");
    printf("\n");
    printf("Output options:\n");
    printf("\n");
    printf("  --ackmate\n");
    printf("  --after LINES\n");
    printf("  --before LINES\n");
    printf("  --context\n");
    printf("  --[no]color\n");
    printf("\n");
}

void print_version() {
    printf("ag version %s\n", AG_VERSION);
}

void init_options() {
    opts.ackmate = 0;
    opts.ackmate_dir_filter = NULL;
    opts.after = 0;
    opts.before = 0;
    opts.casing = CASE_SENSITIVE;
    opts.color = 1;
    opts.context = 0;
    opts.follow_symlinks = 0;
    opts.invert_match = 0;
    opts.literal = 0;
    opts.print_break = 1;
    opts.print_filename_only = 0;
    opts.print_heading = 1;
    opts.recurse_dirs = 1;
}

void cleanup_options() {
    if (opts.ackmate_dir_filter) {
        pcre_free(opts.ackmate_dir_filter);
    }
}

void parse_options(int argc, char **argv) {
    int ch;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;

    init_options();

    int useless = 0;
    int help = 0;
    int version = 0;

    struct option longopts[] = {
        { "ackmate", no_argument, &(opts.ackmate), 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "after", required_argument, NULL, 'A' },
        { "before", required_argument, NULL, 'B' },
        { "break", no_argument, &(opts.print_break), 1 },
        { "nobreak", no_argument, &(opts.print_break), 0 },
        { "color", no_argument, &(opts.color), 1 },
        { "nocolor", no_argument, &(opts.color), 0 },
        { "context", optional_argument, &(opts.context), 2 },
        { "debug", no_argument, NULL, 'D' },
        { "follow", no_argument, &(opts.follow_symlinks), 1 },
        { "invert-match", no_argument, &(opts.invert_match), 1 },
        { "nofollow", no_argument, &(opts.follow_symlinks), 0 },
        { "heading", no_argument, &(opts.print_heading), 1 },
        { "noheading", no_argument, &(opts.print_heading), 0 },
        { "help", no_argument, &help, 1 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "literal", no_argument, &(opts.literal), 1 },
        { "match", no_argument, &useless, 0 },
        { "smart-case", no_argument, &useless, 0 },
        { "nosmart-case", no_argument, &useless, 0 },
        { "version", no_argument, &version, 1 },
        { NULL, 0, NULL, 0 }
    };

    int opt_index = 0;

    if (argc < 2) {
        usage();
        exit(1);
    }

    // TODO: check for insane params. nobody is going to want 5000000 lines of context, for example
    while ((ch = getopt_long(argc, argv, "A:B:C:DfivV", longopts, &opt_index)) != -1) {
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
            case 'D':
                set_log_level(LOG_LEVEL_DEBUG);
                break;
            case 'f':
                opts.print_filename_only = 1;
                break;
            case 'h':
                help = 1;
                break;
            case 'i':
                opts.casing = CASE_INSENSITIVE;
                break;
            case 'v':
                opts.invert_match = 1;
                break;
            case 'V':
                version = 1;
                break;
            case 0: // Long option
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0)
                {
                    opts.ackmate_dir_filter = pcre_compile(optarg, 0, &pcre_err, &pcre_err_offset, NULL);
                    if (opts.ackmate_dir_filter == NULL) {
                        log_err("pcre_compile of ackmate-dir-filter failed at position %i. Error: %s", pcre_err_offset, pcre_err);
                        exit(1);
                    }
                    break;
                }
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

    if (help) {
        usage();
        exit(0);
    }

    if (version) {
        print_version();
        exit(0);
    }

    if (opts.context > 0) {
        opts.before = opts.context;
        opts.after = opts.context;
    }

    if (opts.ackmate) {
        opts.color = 0;
        opts.print_break = 1;
    }

    if (!isatty(fileno(stdout))) {
        opts.color = 0;
    }

    argc -= optind;
    argv += optind;
}
