#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "ignore.h"
#include "options.h"
#include "lang.h"
#include "log.h"
#include "util.h"

#ifdef _WIN32
char* realpath(const char *path, char *resolved_path) {
    char *p;
    char tmp[MAX_PATH + 1];
    strncpy(tmp, path, sizeof(tmp)-1);
    p = tmp;
    while(*p) {
        if (*p == '/') *p = '\\';
        p++;
    }
    return _fullpath(resolved_path, tmp, MAX_PATH);
}
#endif

const char *color_line_number = "\e[1;33m"; /* yellow with black background */
const char *color_match = "\e[30;43m"; /* black with yellow background */
const char *color_path = "\e[1;32m";   /* bold green */

/* TODO: try to obey out_fd? */
void usage() {
    printf("\n");
    printf("Usage: ag [OPTIONS] PATTERN [PATH]\n\n");

    printf("  Recursively search for PATTERN in PATH.\n");
    printf("  Like grep or ack, but faster.\n\n");

    printf("Example:\n  ag -i foo /bar/\n\n");

    printf("\
Output Options:\n\
     --ackmate            Print results in AckMate-parseable format\n\
  -A --after [LINES]      Print lines before match (Default: 2)\n\
  -B --before [LINES]     Print lines after match (Default: 2)\n\
     --[no]break          Print newlines between matches in different files\n\
                          (Enabled by default)\n\
     --[no]color          Print color codes in results (Enabled by default)\n\
     --color-line-number  Color codes for line numbers (Default: 1;33)\n\
     --color-match        Color codes for result match numbers (Default: 30;43)\n\
     --color-path         Color codes for path names (Default: 1;32)\n\
     --column             Print column numbers in results\n\
     --[no]heading\n\
     --line-numbers       Print line numbers even for streams\n\
  -C --context [LINES]    Print lines before and after matches (Default: 2)\n\
     --[no]group          Same as --[no]break --[no]heading\n\
  -g PATTERN              Print filenames matching PATTERN\n\
  -l --files-with-matches Only print filenames that contain matches\n\
                          (don't print the matching lines)\n\
  -L --files-without-matches\n\
                          Only print filenames that don't contain matches\n\
     --no-numbers         Don't print line numbers\n\
     --print-long-lines   Print matches on very long lines (Default: >2k characters)\n\
     --stats              Print stats (files scanned, time taken, etc.)\n\
  -o --only-matching      Print the non-empty matching part of the input\n\
\n\
Search Options:\n\
  -a --all-types          Search all files (doesn't include hidden files\n\
                          or patterns from ignore files)\n\
  -D --debug              Ridiculous debugging (probably not useful)\n\
     --depth NUM          Search up to NUM directories deep (Default: 25)\n\
  -f --follow             Follow symlinks\n\
  -G --file-search-regex  PATTERN Limit search to filenames matching PATTERN\n\
     --hidden             Search hidden files (obeys .*ignore files)\n\
  -i --ignore-case        Match case insensitively\n\
     --ignore PATTERN     Ignore files/directories matching PATTERN\n\
                          (literal file/directory names also allowed)\n\
     --ignore-dir NAME    Alias for --ignore for compatibility with ack.\n\
  -m --max-count NUM      Skip the rest of a file after NUM matches (Default: 10,000)\n\
  -p --path-to-agignore STRING\n\
                          Use .agignore file at STRING\n\
  -Q --literal            Don't parse PATTERN as a regular expression\n\
  -s --case-sensitive     Match case sensitively (Enabled by default)\n\
  -S --smart-case         Match case insensitively unless PATTERN contains\n\
                          uppercase characters\n\
     --search-binary      Search binary files for matches\n\
  -t --all-text           Search all text files (doesn't include hidden files)\n\
  -u --unrestricted       Search all files (ignore .agignore, .gitignore, etc.;\n\
                          searches binary and hidden files as well)\n\
  -U --skip-vcs-ignores   Ignore VCS ignore files\n\
                          (.gitignore, .hgignore, .svnignore; still obey .agignore)\n\
  -v --invert-match\n\
  -w --word-regexp        Only match whole words\n\
  -z --search-zip         Search contents of compressed (e.g., gzip) files\n\
\n");
}

void print_version() {
    printf("ag version %s\n", PACKAGE_VERSION);
}

void init_options() {
    memset(&opts, 0, sizeof(opts));
    opts.casing = CASE_SENSITIVE;
#ifdef _WIN32
    opts.color = getenv("ANSICON") ? TRUE : FALSE;
#else
    opts.color = TRUE;
#endif
    opts.max_matches_per_file = 10000;
    opts.max_search_depth = 25;
    opts.print_break = TRUE;
    opts.print_heading = TRUE;
    opts.print_line_numbers = TRUE;
    opts.recurse_dirs = TRUE;
    opts.color_path = ag_strdup(color_path);
    opts.color_match = ag_strdup(color_match);
    opts.color_line_number = ag_strdup(color_line_number);
}

void cleanup_options() {
    free(opts.color_path);
    free(opts.color_match);
    free(opts.color_line_number);

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

void parse_options(int argc, char **argv, char **base_paths[], char **paths[]) {
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
    struct stat statbuf;
    int rv;

    size_t longopts_len, full_len;
    option_t* longopts;
    char *lang_regex = NULL;

    init_options();

    option_t base_longopts[] = {
        { "ackmate", no_argument, &opts.ackmate, 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "after", required_argument, NULL, 'A' },
        { "all-text", no_argument, NULL, 't' },
        { "all-types", no_argument, NULL, 'a' },
        { "before", required_argument, NULL, 'B' },
        { "break", no_argument, &opts.print_break, 1 },
        { "case-sensitive", no_argument, NULL, 's' },
        { "color", no_argument, &opts.color, 1 },
        { "color-path", required_argument, NULL, 0 },
        { "color-match", required_argument, NULL, 0 },
        { "color-line-number", required_argument, NULL, 0 },
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
        { "ignore-dir", required_argument, NULL, 0 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "invert-match", no_argument, &opts.invert_match, 1 },
        { "line-numbers", no_argument, &opts.print_line_numbers, 2 },
        { "literal", no_argument, NULL, 'Q' },
        { "match", no_argument, &useless, 0 },
        { "max-count", required_argument, NULL, 'm' },
        { "only-matching", no_argument, &opts.only_matching, 'o' },
        { "no-numbers", no_argument, NULL, 0 },
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
        { "search-zip", no_argument, &opts.search_zip_files, 1 },
        { "search-files", no_argument, &opts.search_stream, 0 },
        { "silent", no_argument, NULL, 0 },
        { "skip-vcs-ignores", no_argument, NULL, 'U' },
        { "smart-case", no_argument, NULL, 'S' },
        { "stats", no_argument, &opts.stats, 1 },
        { "unrestricted", no_argument, NULL, 'u' },
        { "version", no_argument, &version, 1 },
        { "word-regexp", no_argument, NULL, 'w' },
        { "workers", required_argument, NULL, 0 },
    };

    longopts_len = (sizeof(base_longopts) / sizeof(option_t));
    full_len = (longopts_len + LANG_COUNT + 1);
    longopts = ag_malloc(full_len * sizeof(option_t));
    memcpy(longopts, base_longopts, sizeof(base_longopts));

    for (i = 0; i < LANG_COUNT; i++) {
        option_t opt = { langs[i].name, no_argument, NULL, 0 };
        longopts[i + longopts_len] = opt;
    }
    longopts[full_len-1] = (option_t){ NULL, 0, NULL, 0 };

    if (argc < 2) {
        usage();
        cleanup_ignore(root_ignores);
        cleanup_options();
        exit(1);
    }

    rv = fstat(fileno(stdin), &statbuf);
    if (rv != 0) {
        die("Error fstat()ing stdin");
    }
    if (S_ISFIFO(statbuf.st_mode)) {
        opts.search_stream = 1;
    }

    /* If we're not outputting to a terminal. change output to:
        * turn off colors
        * print filenames on every line
     */
    if (!isatty(fileno(stdout))) {
        opts.color = 0;
        group = 0;

        /* Don't search the file that stdout is redirected to */
        rv = fstat(fileno(stdout), &statbuf);
        if (rv != 0) {
            die("Error fstat()ing stdout");
        }
        opts.stdout_inode = statbuf.st_ino;
    }

    while ((ch = getopt_long(argc, argv, "A:aB:C:DG:g:fhiLlm:nop:QRrSsvVtuUwz", longopts, &opt_index)) != -1) {
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
                compile_study(&opts.file_search_regex, &opts.file_search_regex_extra, optarg, opts.casing & PCRE_CASELESS, 0);
                opts.casing = CASE_SENSITIVE;
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
            case 'o':
                opts.only_matching = TRUE;
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
            case 'z':
                opts.search_zip_files = 1;
                break;
            case 0: /* Long option */
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0) {
                    compile_study(&opts.ackmate_dir_filter, &opts.ackmate_dir_filter_extra, optarg, 0, 0);
                    break;
                } else if (strcmp(longopts[opt_index].name, "depth") == 0) {
                    opts.max_search_depth = atoi(optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "no-numbers") == 0) {
                    opts.print_line_numbers = FALSE;
                    break;
                } else if (strcmp(longopts[opt_index].name, "ignore-dir") == 0) {
                    add_ignore_pattern(root_ignores, optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "ignore") == 0) {
                    add_ignore_pattern(root_ignores, optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "nopager") == 0) {
                    out_fd = stdout;
                    opts.pager = NULL;
                    break;
                } else if (strcmp(longopts[opt_index].name, "only-matching") == 0) {
                    opts.only_matching = TRUE;
                } else if (strcmp(longopts[opt_index].name, "pager") == 0) {
                    opts.pager = optarg;
                    break;
                } else if (strcmp(longopts[opt_index].name, "workers") == 0) {
                    opts.workers = atoi(optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-line-number") == 0) {
                    free(opts.color_line_number);
                    ag_asprintf(&opts.color_line_number, "\e[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-match") == 0) {
                    free(opts.color_match);
                    ag_asprintf(&opts.color_match, "\e[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-path") == 0) {
                    free(opts.color_path);
                    ag_asprintf(&opts.color_path, "\e[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "silent") == 0) {
                    set_log_level(LOG_LEVEL_NONE);
                    break;
                }

                /* Continue to usage if we don't recognize the option */
                if (longopts[opt_index].flag != 0) {
                    break;
                }

                for (i = 0; i < LANG_COUNT; i++) {
                    if (strcmp(longopts[opt_index].name, langs[i].name) == 0) {
                        lang_regex = make_lang_regex(langs[i].extensions);
                        compile_study(&opts.file_search_regex, &opts.file_search_regex_extra, lang_regex, 0, 0);
                        break;
                    }
                }
                if (lang_regex) {
                    free(lang_regex);
                    lang_regex = NULL;
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
        ag_asprintf(&ignore_file_path, "%s/%s", home_dir, ignore_pattern_files[0]);
        load_ignore_patterns(root_ignores, ignore_file_path);
        free(ignore_file_path);
    }

    if (!opts.skip_vcs_ignores) {
        FILE *gitconfig_file = NULL;
        size_t buf_len = 0;
        char *gitconfig_res = NULL;

        gitconfig_file = popen("git config -z --get core.excludesfile", "r");
        if (gitconfig_file != NULL) {
            do {
                gitconfig_res = ag_realloc(gitconfig_res, buf_len + 65);
                buf_len += fread(gitconfig_res + buf_len, 1, 64, gitconfig_file);
            } while (!feof(gitconfig_file) && buf_len > 0 && buf_len % 64 == 0);
            gitconfig_res[buf_len] = '\0';
            load_ignore_patterns(root_ignores, gitconfig_res);
            free(gitconfig_res);
            pclose(gitconfig_file);
        }
    }

    if (opts.context > 0) {
        opts.before = opts.context;
        opts.after = opts.context;
    }

    if (opts.only_matching && (opts.before || opts.after)) {
        log_err("When --only-matching is specified, --context, --before and --after have no effect");
        opts.context = opts.before = opts.after = 0;
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
        if (opts.print_line_numbers != 2)
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
    char *tmp = NULL;
    opts.paths_len = argc;
    if (argc > 0) {
        *paths = ag_calloc(sizeof(char*), argc + 1);
        *base_paths = ag_calloc(sizeof(char*), argc + 1);
        for (i = 0; i < argc; i++) {
            path = ag_strdup(argv[i]);
            path_len = strlen(path);
            /* kill trailing slash */
            if (path_len > 1 && path[path_len - 1] == '/') {
              path[path_len - 1] = '\0';
            }
            (*paths)[i] = path;
            tmp = ag_malloc(PATH_MAX);
            (*base_paths)[i] = realpath(path, tmp);
        }
        /* Make sure we search these paths instead of stdin. */
        opts.search_stream = 0;
    } else {
        path = ag_strdup(".");
        *paths = ag_malloc(sizeof(char*) * 2);
        *base_paths = ag_malloc(sizeof(char*) * 2);
        (*paths)[0] = path;
        tmp = ag_malloc(PATH_MAX);
        (*base_paths)[0] = realpath(path, tmp);
        i = 1;
    }
    (*paths)[i] = NULL;
    (*base_paths)[i] = NULL;
}
