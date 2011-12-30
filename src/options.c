#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xlocale.h>

#include "options.h"
#include "log.h"
#include "version.h"

/* TODO: printf()ing this is not going to scale */
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
    printf("  -G, --file-search-regex\n");
    printf("  --literal\n");
    printf("\n");
    printf("Output options:\n");
    printf("\n");
    printf("  --ackmate\n");
    printf("  --after LINES\n");
    printf("  --before LINES\n");
    printf("  --column\n");
    printf("  --context\n");
    printf("  --[no]color\n");
    printf("  --[no]group\n");
    printf("\n");
}

void print_version() {
    printf("ag version %s\n", AG_VERSION);
}

void init_options() {
    memset(&opts, 0, sizeof(&opts));
    opts.casing = CASE_SENSITIVE;
    opts.color = TRUE;
    opts.print_break = TRUE;
    opts.print_heading = TRUE;
    opts.recurse_dirs = TRUE;
}

void cleanup_options() {
    if (opts.ackmate_dir_filter) {
        pcre_free(opts.ackmate_dir_filter);
        pcre_free(opts.ackmate_dir_filter_extra);
    }
    if (opts.file_search_regex) {
        pcre_free(opts.file_search_regex);
        pcre_free(opts.file_search_regex_extra);
    }
}

void parse_options(int argc, char **argv, char **query, char **path) {
    int ch;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    int path_len = 0;
    int useless = 0;
    int group = 1;
    int help = 0;
    int version = 0;
    int opt_index = 0;

    init_options();

    struct option longopts[] = {
        { "ackmate", no_argument, &(opts.ackmate), 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "after", required_argument, NULL, 'A' },
        { "before", required_argument, NULL, 'B' },
        { "break", no_argument, &(opts.print_break), 1 },
        { "nobreak", no_argument, &(opts.print_break), 0 },
        { "color", no_argument, &(opts.color), 1 },
        { "column", no_argument, &(opts.column), 1 },
        { "nocolor", no_argument, &(opts.color), 0 },
        { "context", optional_argument, &(opts.context), 2 },
        { "debug", no_argument, NULL, 'D' },
        { "follow", no_argument, &(opts.follow_symlinks), 1 },
        { "file-search-regex", required_argument, NULL, 'G' },
        { "group", no_argument, &(group), 1 },
        { "nogroup", no_argument, &(group), 0 },
        { "invert-match", no_argument, &(opts.invert_match), 1 },
        { "nofollow", no_argument, &(opts.follow_symlinks), 0 },
        { "heading", no_argument, &(opts.print_heading), 1 },
        { "noheading", no_argument, &(opts.print_heading), 0 },
        { "help", no_argument, &help, 1 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "literal", no_argument, &(opts.literal), 1 },
        { "match", no_argument, &useless, 0 },
        { "search-binary", no_argument, &(opts.search_binary_files), 1 },
        { "smart-case", no_argument, &useless, 0 },
        { "nosmart-case", no_argument, &useless, 0 },
        { "stats", no_argument, &(opts.stats), 1 },
        { "version", no_argument, &version, 1 },
        { NULL, 0, NULL, 0 }
    };

    if (argc < 2) {
        usage();
        exit(1);
    }

    /* TODO: check for insane params. nobody is going to want 5000000 lines of context, for example */
    while ((ch = getopt_long(argc, argv, "A:B:C:G:DfivV", longopts, &opt_index)) != -1) {
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
            case 'G':
                opts.file_search_regex = pcre_compile(optarg, 0, &pcre_err, &pcre_err_offset, NULL);
                if (opts.file_search_regex == NULL) {
                  log_err("pcre_compile of file-search-regex failed at position %i. Error: %s", pcre_err_offset, pcre_err);
                  exit(1);
                }

                opts.file_search_regex_extra = pcre_study(opts.file_search_regex, 0, &pcre_err);
                if (opts.file_search_regex_extra == NULL) {
                  log_err("pcre_study of file-search-regex failed. Error: %s", pcre_err);
                  exit(1);
                }
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
            case 0: /* Long option */
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0)
                {
                    opts.ackmate_dir_filter = pcre_compile(optarg, 0, &pcre_err, &pcre_err_offset, NULL);
                    if (opts.ackmate_dir_filter == NULL) {
                        log_err("pcre_compile of ackmate-dir-filter failed at position %i. Error: %s", pcre_err_offset, pcre_err);
                        exit(1);
                    }
                    opts.ackmate_dir_filter_extra = pcre_study(opts.ackmate_dir_filter, 0, &pcre_err);
                    if (opts.ackmate_dir_filter_extra == NULL) {
                      log_err("pcre_study of ackmate-dir-filter failed. Error: %s", pcre_err);
                      exit(1);
                    }
                    break;
                }
                /* Continue to usage if we don't recognize the option */
                if (longopts[opt_index].flag != 0) {
                    break;
                }
                log_err("option %s does not take a value", longopts[opt_index].name);
            default:
                usage();
                exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    if (group) {
        opts.print_heading = 1;
        opts.print_break = 1;
    }
    else {
        opts.print_heading = 0;
        opts.print_break = 0;
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

    opts.query = strdup(argv[0]);
    opts.query_len = strlen(opts.query);
    *query = opts.query;

    if (argc > 1) {
      *path = strdup(argv[1]);
      path_len = strlen(*path);
      /* kill trailing slash */
      if (path_len > 0 && (*path)[path_len - 1] == '/') {
        (*path)[path_len - 1] = '\0';
      }
    }
    else {
      *path = strdup(".");
    }
}
