#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "ignore.h"
#include "options.h"
#include "log.h"
#include "util.h"

/* TODO: printf()ing this is not going to scale */
void usage() {
    printf("Usage: ag [OPTIONS] PATTERN [PATH]\n\
\n\
Recursively search for PATTERN in PATH.\n\
Like grep or ack, but faster.\n\
\n\
Example: ag -i foo /bar/\n\
\n\
Search options:\n\
\n\
--ackmate: Output results in a format parseable by AckMate.\n\
-A --after [LINES]: Print lines before match. Defaults to 2.\n\
-B --before [LINES]: Print lines after match. Defaults to 2.\n\
--[no]break: Print a newline between matches in different files. Enabled by default.\n\
--[no]color: Print color codes in results. Enabled by default.\n\
--column: Print column numbers in results.\n\
-C --context [LINES]: Print lines before and after matches. Defaults to 2.\n\
-D --debug: Ridiculous debugging. Probably not useful.\n\
--depth NUM: Search up to NUM directories deep. Default is 25.\n\
-f --follow: Follow symlinks.\n\
--[no]group: Same as --[no]break --[no]heading\n\
-G, --file-search-regex\n\
-i, --ignore-case\n\
--invert-match\n\
--[no]heading\n\
-l --files-with-matches: Only print filenames containing matches, not matching lines.\n\
--literal: Do not parse PATTERN as a regular expression. Try to match it literally.\n\
-m --max-count NUM: Skip the rest of a file after NUM matches. Default is 10,000.\n\
--print-long-lines: Print matches on very long lines (> 2k characters by default)\n\
--search-binary: Search binary files for matches.\n\
--stats: Print stats (files scanned, time taken, etc)\n\
\n");
}

void print_version() {
    printf("ag version %s\n", PACKAGE_VERSION);
}

void init_options() {
    memset(&opts, 0, sizeof(opts));
    opts.casing = CASE_SENSITIVE;
    opts.color = TRUE;
    opts.max_matches_per_file = 10000;
    opts.max_search_depth = 25;
    opts.print_break = TRUE;
    opts.print_heading = TRUE;
    opts.print_line_numbers = TRUE;
    opts.recurse_dirs = TRUE;
}

void cleanup_options() {
    if (opts.ackmate_dir_filter) {
        pcre_free(opts.ackmate_dir_filter);
        pcre_free(opts.ackmate_dir_filter_extra); /* Using pcre_free_study here segfaults on some versions of PCRE */
    }

    if (opts.file_search_regex) {
        pcre_free(opts.file_search_regex);
        pcre_free(opts.file_search_regex_extra); /* Using pcre_free_study here segfaults on some versions of PCRE */
    }
}

void parse_options(int argc, char **argv, char **paths[]) {
    int ch;
    int i;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    int path_len = 0;
    int useless = 0;
    int group = 1;
    int help = 0;
    int version = 0;
    int opt_index = 0;
    const char *home_dir = getenv("HOME");
    char *ignore_file_path = NULL;

    init_options();

    struct option longopts[] = {
        { "ackmate", no_argument, &(opts.ackmate), 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "after", required_argument, NULL, 'A' },
        { "all-types", no_argument, NULL, 'a' },
        { "before", required_argument, NULL, 'B' },
        { "break", no_argument, &(opts.print_break), 1 },
        { "nobreak", no_argument, &(opts.print_break), 0 },
        { "color", no_argument, &(opts.color), 1 },
        { "nocolor", no_argument, &(opts.color), 0 },
        { "column", no_argument, &(opts.column), 1 },
        { "context", optional_argument, &(opts.context), 2 },
        { "debug", no_argument, NULL, 'D' },
        { "depth", required_argument, NULL, 0 },
        { "follow", no_argument, &(opts.follow_symlinks), 1 },
        { "file-search-regex", required_argument, NULL, 'G' },
        { "group", no_argument, &(group), 1 },
        { "nogroup", no_argument, &(group), 0 },
        { "hidden", no_argument, &(opts.search_hidden_files), 1 },
        { "invert-match", no_argument, &(opts.invert_match), 1 },
        { "nofollow", no_argument, &(opts.follow_symlinks), 0 },
        { "heading", no_argument, &(opts.print_heading), 1 },
        { "noheading", no_argument, &(opts.print_heading), 0 },
        { "no-recurse", no_argument, NULL, 'n' },
        { "help", no_argument, NULL, 'h' },
        { "ignore-case", no_argument, NULL, 'i' },
        { "files-with-matches", no_argument, NULL, 'l' },
        { "literal", no_argument, &(opts.literal), 1 },
        { "match", no_argument, &useless, 0 },
        { "max-count", required_argument, NULL, 'm' },
        { "parallel", no_argument, &(opts.parallel), 1},
        { "print-long-lines", no_argument, &(opts.print_long_lines), 1 },
        { "search-binary", no_argument, &(opts.search_binary_files), 1 },
        { "search-files", no_argument, &(opts.search_stream), 0 },
        { "smart-case", no_argument, &useless, 0 },
        { "nosmart-case", no_argument, &useless, 0 },
        { "stats", no_argument, &(opts.stats), 1 },
        { "unrestricted", no_argument, NULL, 'u' },
        { "version", no_argument, &version, 1 },
        { NULL, 0, NULL, 0 }
    };

    if (argc < 2) {
        usage();
        exit(1);
    }

    /* stdin isn't a tty. something's probably being piped to ag */
    if (!isatty(fileno(stdin))) {
        opts.search_stream = 1;
    }

    /* If we're not outputting to a terminal. change output to:
        * turn off colors
        * print filenames on every line
     */
    if (!isatty(fileno(stdout))) {
        opts.color = 0;
        group = 0;
    }

    while ((ch = getopt_long(argc, argv, "A:aB:C:DG:fhilm:nvVu", longopts, &opt_index)) != -1) {
        switch (ch) {
            case 'A':
                opts.after = atoi(optarg);
                break;
            case 'a':
                opts.search_all_files = 1;
                opts.search_binary_files = 1;
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
                opts.follow_symlinks = 1;
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
            case 'l':
                opts.print_filename_only = 1;
                break;
            case 'm':
                opts.max_matches_per_file = atoi(optarg);
                break;
            case 'n':
                opts.recurse_dirs = 0;
                break;
            case 'u':
                opts.search_binary_files = 1;
                opts.search_unrestricted = 1;
                opts.search_hidden_files = 1;
                break;
            case 'v':
                opts.invert_match = 1;
                break;
            case 'V':
                version = 1;
                break;
            case 0: /* Long option */
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0) {
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
                else if (strcmp(longopts[opt_index].name, "depth") == 0) {
                    opts.max_search_depth = atoi(optarg);
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

    if (help) {
        usage();
        exit(0);
    }

    if (version) {
        print_version();
        exit(0);
    }

    if (argc == 0) {
        log_err("What do you want to search for?");
        exit(1);
    }

    if (home_dir) {
        log_debug("Found user's home dir: %s", home_dir);
        size_t path_length = (size_t)(strlen(home_dir) + strlen(ignore_pattern_files[0]) + 2);
        ignore_file_path = malloc(path_length);
        strlcpy(ignore_file_path, home_dir, path_length);
        strlcat(ignore_file_path, "/", path_length);
        strlcat(ignore_file_path, ignore_pattern_files[0], path_length);

        load_ignore_patterns(ignore_file_path);

        free(ignore_file_path);
    }

    if (opts.context > 0) {
        opts.before = opts.context;
        opts.after = opts.context;
    }

    if (opts.ackmate) {
        opts.color = 0;
        opts.print_break = 1;
        group = 1;
        opts.search_stream = 0;
    }

    if (opts.parallel) {
        opts.search_stream = 0;
    }

    if (opts.print_heading == 0 || opts.print_break == 0) {
        goto skip_group;
    }

    if (group) {
        opts.print_heading = 1;
        opts.print_break = 1;
    }
    else {
        opts.print_heading = 0;
        opts.print_break = 0;
    }

    skip_group:

    if (opts.search_stream) {
        opts.print_break = 0;
        opts.print_heading = 0;
        opts.print_line_numbers = 0;
    }

    opts.query = strdup(argv[0]);
    opts.query_len = strlen(opts.query);

    log_debug("Query is %s", opts.query);

    if (opts.query_len == 0) {
        log_err("Error: No query. What do you want to search for?");
        exit(1);
    }

    char *path = NULL;
    opts.paths_len = argc - 1;
    if (argc > 1) {
        *paths = malloc(sizeof(char*) * argc);
        for (i = 1; i < argc; i++) {
            path = strdup(argv[i]);
            path_len = strlen(path);
            /* kill trailing slash */
            if (path_len > 0 && path[path_len - 1] == '/') {
              path[path_len - 1] = '\0';
            }
            (*paths)[i-1] = path;
        }
        (*paths)[i-1] = NULL;
    }
    else {
        path = strdup(".");
        *paths = malloc(sizeof(char*) * 2);
        (*paths)[0] = path;
        (*paths)[1] = NULL;
    }
}
