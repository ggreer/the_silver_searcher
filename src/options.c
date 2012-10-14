#include <errno.h>
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
-a --all-types: Search all files. This doesn't include hidden files, and also doesn't respect any ignore files.\n\
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
-g PATTERN: Print filenames that match PATTERN\n\
-G, --file-search-regex PATTERN: Only search file names matching PATTERN\n\
--[no]heading\n\
--hidden: Search hidden files. This option obeys ignore files.\n\
-i, --ignore-case: Match case insensitively\n\
--ignore PATTERN: Ignore files/directories matching this pattern. Literal file and directory names are also allowed.\n\
-l --files-with-matches: Only print filenames containing matches, not matching lines.\n\
-L --files-without-matches: Only print filenames that don't contain matches.\n\
-m --max-count NUM: Skip the rest of a file after NUM matches. Default is 10,000.\n\
-p --path-to-agignore STRING: Provide a path to a specific .agignore file\n\
--print-long-lines: Print matches on very long lines (> 2k characters by default)\n\
-Q --literal: Do not parse PATTERN as a regular expression. Try to match it literally.\n\
-s --case-sensitive: Match case sensitively. Enabled by default.\n\
-S --smart-case: Match case sensitively if there are any uppercase letters in PATTERN, or case insensitively otherwise.\n\
--search-binary: Search binary files for matches.\n\
--stats: Print stats (files scanned, time taken, etc)\n\
-t --all-text: Search all text files. This doesn't include hidden files.\n\
-u --unrestricted: Search *all* files. This ignores .agignore, .gitignore, etc. It searches binary and hidden files as well.\n\
-U --skip-vcs-ignores: Ignore VCS ignore files (.gitigore, .hgignore, svn:ignore), but still use .agignore.\n\
-v --invert-match\n\
-w --word-regexp: Only match whole words.\n\
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
    if (opts.query) {
        free(opts.query);
    }

    pcre_free(opts.re);
    if (opts.re_extra) {
         /* Using pcre_free_study on pcre_extra* can segfault on some versions of PCRE */
        pcre_free(opts.re_extra);
    }

    if (opts.ackmate_dir_filter) {
        pcre_free(opts.ackmate_dir_filter);
    }
    if (opts.ackmate_dir_filter_extra) {
        pcre_free(opts.ackmate_dir_filter_extra);
    }

    if (opts.file_search_regex) {
        pcre_free(opts.file_search_regex);
    }
    if (opts.file_search_regex_extra) {
        pcre_free(opts.file_search_regex_extra);
    }
}

void parse_options(int argc, char **argv, char **paths[]) {
    int ch;
    int i;
    int path_len = 0;
    int useless = 0;
    int group = 1;
    int help = 0;
    int version = 0;
    int opt_index = 0;
    const char *home_dir = getenv("HOME");
    char *ignore_file_path = NULL;
    int needs_query = 1;

    init_options();

    struct option longopts[] = {
        { "ackmate", no_argument, &opts.ackmate, 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "after", required_argument, NULL, 'A' },
        { "all-text", no_argument, NULL, 't' },
        { "all-types", no_argument, NULL, 'a' },
        { "before", required_argument, NULL, 'B' },
        { "break", no_argument, &opts.print_break, 1 },
        { "case-sensitive", no_argument, NULL, 's' },
        { "color", no_argument, &opts.color, 1 },
        { "column", no_argument, &opts.column, 1 },
        { "context", optional_argument, NULL, 'C' },
        { "debug", no_argument, NULL, 'D' },
        { "depth", required_argument, NULL, 0 },
        { "file-search-regex", required_argument, NULL, 'G' },
        { "files-with-matches", no_argument, NULL, 'l' },
        { "files-without-matches", no_argument, NULL, 'L' },
        { "follow", no_argument, &opts.follow_symlinks, 1 },
        { "group", no_argument, &group, 1 },
        { "heading", no_argument, &opts.print_heading, 1 },
        { "help", no_argument, NULL, 'h' },
        { "hidden", no_argument, &opts.search_hidden_files, 1 },
        { "ignore", required_argument, NULL, 0 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "invert-match", no_argument, &opts.invert_match, 1 },
        { "literal", no_argument, NULL, 'Q' },
        { "match", no_argument, &useless, 0 },
        { "max-count", required_argument, NULL, 'm' },
        { "no-recurse", no_argument, NULL, 'n' },
        { "nobreak", no_argument, &opts.print_break, 0 },
        { "nocolor", no_argument, &opts.color, 0 },
        { "nofollow", no_argument, &opts.follow_symlinks, 0 },
        { "nogroup", no_argument, &group, 0 },
        { "noheading", no_argument, &opts.print_heading, 0 },
        { "nopager", no_argument, NULL, 0 },
        { "pager", required_argument, NULL, 0 },
        { "parallel", no_argument, &opts.parallel, 1},
        { "path-to-agignore", required_argument, NULL, 'p'},
        { "print-long-lines", no_argument, &opts.print_long_lines, 1 },
        { "recurse", no_argument, NULL, 'r' },
        { "search-binary", no_argument, &opts.search_binary_files, 1 },
        { "search-files", no_argument, &opts.search_stream, 0 },
        { "skip-vcs-ignores", no_argument, NULL, 'U' },
        { "smart-case", no_argument, NULL, 'S' },
        { "stats", no_argument, &opts.stats, 1 },
        { "unrestricted", no_argument, NULL, 'u' },
        { "version", no_argument, &version, 1 },
        { "word-regexp", no_argument, NULL, 'w' },
        { "workers", required_argument, NULL, 0 },
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

    while ((ch = getopt_long(argc, argv, "A:aB:C:DG:g:fhiLlm:np:QRrSsvVtuUw", longopts, &opt_index)) != -1) {
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
                if (optarg) {
                    opts.context = atoi(optarg);
                    if (opts.context == 0 && errno == EINVAL) {
                        /* This arg must be the search string instead of the context length */
                        optind--;
                        opts.context = DEFAULT_CONTEXT_LEN;
                    }
                } else {
                    opts.context = DEFAULT_CONTEXT_LEN;
                }
                break;
            case 'D':
                set_log_level(LOG_LEVEL_DEBUG);
                break;
            case 'f':
                opts.follow_symlinks = 1;
                break;
            case 'g':
                needs_query = 0;
                opts.match_files = 1;
                /* Fall through and build regex */
            case 'G':
                compile_study(&opts.file_search_regex, &opts.file_search_regex_extra, optarg, 0, 0);
                break;
            case 'h':
                help = 1;
                break;
            case 'i':
                opts.casing = CASE_INSENSITIVE;
                break;
            case 'L':
                opts.invert_match = 1;
                /* fall through */
            case 'l':
                opts.print_filename_only = 1;
                break;
            case 'm':
                opts.max_matches_per_file = atoi(optarg);
                break;
            case 'n':
                opts.recurse_dirs = 0;
                break;
            case 'p':
                opts.path_to_agignore = optarg;
                break;
            case 'Q':
                opts.literal = 1;
                break;
            case 'R':
            case 'r':
                opts.recurse_dirs = 1;
                break;
            case 'S':
                opts.casing = CASE_SMART;
                break;
            case 's':
                opts.casing = CASE_SENSITIVE;
                break;
            case 't':
                opts.search_all_files = 1;
                break;
            case 'u':
                opts.search_binary_files = 1;
                opts.search_all_files = 1;
                opts.search_hidden_files = 1;
                break;
            case 'U':
                opts.skip_vcs_ignores = 1;
                break;
            case 'v':
                opts.invert_match = 1;
                break;
            case 'V':
                version = 1;
                break;
            case 'w':
                opts.word_regexp = 1;
                break;
            case 0: /* Long option */
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0) {
                    compile_study(&opts.ackmate_dir_filter, &opts.ackmate_dir_filter_extra, optarg, 0, 0);
                    break;
                } else if (strcmp(longopts[opt_index].name, "depth") == 0) {
                    opts.max_search_depth = atoi(optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "ignore") == 0) {
                    add_ignore_pattern(root_ignores, optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "nopager") == 0) {
                    out_fd = stdout;
                    opts.pager = NULL;
                    break;
                } else if (strcmp(longopts[opt_index].name, "pager") == 0) {
                    opts.pager = optarg;
                    break;
                } else if (strcmp(longopts[opt_index].name, "workers") == 0) {
                    opts.workers = atoi(optarg);
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

    if (opts.pager) {
        out_fd = popen(opts.pager, "w");
        if (!out_fd) {
            perror("Failed to run pager");
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

    if (needs_query && argc == 0) {
        log_err("What do you want to search for?");
        exit(1);
    }

    if (home_dir && !opts.search_all_files) {
        log_debug("Found user's home dir: %s", home_dir);
        asprintf(&ignore_file_path, "%s/%s", home_dir, ignore_pattern_files[0]);
        load_ignore_patterns(root_ignores, ignore_file_path);
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
    } else {
        opts.print_heading = 0;
        opts.print_break = 0;
    }

    skip_group:;

    if (opts.search_stream) {
        opts.print_break = 0;
        opts.print_heading = 0;
        opts.print_line_numbers = 0;
    }

    if (needs_query) {
        opts.query = ag_strdup(argv[0]);
        argc--;
        argv++;
    } else {
        opts.query = ag_strdup(".");
    }
    opts.query_len = strlen(opts.query);

    log_debug("Query is %s", opts.query);

    if (opts.query_len == 0) {
        log_err("Error: No query. What do you want to search for?");
        exit(1);
    }

    if (!is_regex(opts.query)) {
        opts.literal = 1;
    }

    char *path = NULL;
    opts.paths_len = argc;
    if (argc > 0) {
        *paths = ag_calloc(sizeof(char*), argc + 1);
        for (i = 0; i < argc; i++) {
            path = ag_strdup(argv[i]);
            path_len = strlen(path);
            /* kill trailing slash */
            if (path_len > 1 && path[path_len - 1] == '/') {
              path[path_len - 1] = '\0';
            }
            (*paths)[i] = path;
        }
        (*paths)[i] = NULL;
        /* Make sure we search these paths instead of stdin. */
        opts.search_stream = 0;
    } else {
        path = ag_strdup(".");
        *paths = ag_malloc(sizeof(char*) * 2);
        (*paths)[0] = path;
        (*paths)[1] = NULL;
    }
}
