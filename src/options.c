#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "ignore.h"
#include "lang.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

const char *color_line_number = "\033[1;33m"; /* bold yellow */
const char *color_match = "\033[30;43m";      /* black with yellow background */
const char *color_path = "\033[1;32m";        /* bold green */

cli_options opts;

/* TODO: try to obey out_fd? */
void usage(void) {
    printf("\n");
    printf("Usage: ag [FILE-TYPE] [OPTIONS] PATTERN [PATH]\n\n");

    printf("  Recursively search for PATTERN in PATH.\n");
    printf("  Like grep or ack, but faster.\n\n");

    printf("Example:\n  ag -i foo /bar/\n\n");

    printf("\
Output Options:\n\
     --ackmate            Print results in AckMate-parseable format\n\
  -A --after [LINES]      Print lines after match (Default: 2)\n\
  -B --before [LINES]     Print lines before match (Default: 2)\n\
     --[no]break          Print newlines between matches in different files\n\
                          (Enabled by default)\n\
  -c --count              Only print the number of matches in each file.\n\
                          (This often differs from the number of matching lines)\n\
     --[no]color          Print color codes in results (Enabled by default)\n\
     --color-line-number  Color codes for line numbers (Default: 1;33)\n\
     --color-match        Color codes for result match numbers (Default: 30;43)\n\
     --color-path         Color codes for path names (Default: 1;32)\n\
");
#ifdef _WIN32
    printf("\
     --color-win-ansi     Use ansi colors on Windows even where we can use native\n\
                          (pager/pipe colors are ansi regardless) (Default: off)\n\
");
#endif
    printf("\
     --column             Print column numbers in results\n\
     --[no]filename       Print file names (Enabled unless searching a single file)\n\
  -H --[no]heading        Print file names before each file's matches\n\
                          (Enabled by default)\n\
  -C --context [LINES]    Print lines before and after matches (Default: 2)\n\
     --[no]group          Same as --[no]break --[no]heading\n\
  -g --filename-pattern PATTERN\n\
                          Print filenames matching PATTERN\n\
  -l --files-with-matches Only print filenames that contain matches\n\
                          (don't print the matching lines)\n\
  -L --files-without-matches\n\
                          Only print filenames that don't contain matches\n\
     --print-all-files    Print headings for all files searched, even those that\n\
                          don't contain matches\n\
     --[no]numbers        Print line numbers. Default is to omit line numbers\n\
                          when searching streams\n\
  -o --only-matching      Prints only the matching part of the lines\n\
     --print-long-lines   Print matches on very long lines (Default: >2k characters)\n\
     --passthrough        When searching a stream, print all lines even if they\n\
                          don't match\n\
     --silent             Suppress all log messages, including errors\n\
     --stats              Print stats (files scanned, time taken, etc.)\n\
     --stats-only         Print stats and nothing else.\n\
                          (Same as --count when searching a single file)\n\
     --vimgrep            Print results like vim's :vimgrep /pattern/g would\n\
                          (it reports every match on the line)\n\
  -0 --null --print0      Separate filenames with null (for 'xargs -0')\n\
\n\
Search Options:\n\
  -a --all-types          Search all files (doesn't include hidden files\n\
                          or patterns from ignore files)\n\
  -D --debug              Ridiculous debugging (probably not useful)\n\
     --depth NUM          Search up to NUM directories deep (Default: 25)\n\
  -f --follow             Follow symlinks\n\
  -F --fixed-strings      Alias for --literal for compatibility with grep\n\
  -G --file-search-regex  PATTERN Limit search to filenames matching PATTERN\n\
     --hidden             Search hidden files (obeys .*ignore files)\n\
  -i --ignore-case        Match case insensitively\n\
     --ignore PATTERN     Ignore files/directories matching PATTERN\n\
                          (literal file/directory names also allowed)\n\
     --ignore-dir NAME    Alias for --ignore for compatibility with ack.\n\
  -m --max-count NUM      Skip the rest of a file after NUM matches (Default: 10,000)\n\
     --one-device         Don't follow links to other devices.\n\
  -p --path-to-ignore STRING\n\
                          Use .ignore file at STRING\n\
  -Q --literal            Don't parse PATTERN as a regular expression\n\
  -s --case-sensitive     Match case sensitively\n\
  -S --smart-case         Match case insensitively unless PATTERN contains\n\
                          uppercase characters (Enabled by default)\n\
     --search-binary      Search binary files for matches\n\
  -t --all-text           Search all text files (doesn't include hidden files)\n\
  -u --unrestricted       Search all files (ignore .ignore, .gitignore, etc.;\n\
                          searches binary and hidden files as well)\n\
  -U --skip-vcs-ignores   Ignore VCS ignore files\n\
                          (.gitignore, .hgignore; still obey .ignore)\n\
  -v --invert-match\n\
  -w --word-regexp        Only match whole words\n\
  -W --width NUM          Truncate match lines after NUM characters\n\
  -z --search-zip         Search contents of compressed (e.g., gzip) files\n\
\n");
    printf("File Types:\n\
The search can be restricted to certain types of files. Example:\n\
  ag --html needle\n\
  - Searches for 'needle' in files with suffix .htm, .html, .shtml or .xhtml.\n\
\n\
For a list of supported file types run:\n\
  ag --list-file-types\n\n\
ag was originally created by Geoff Greer. More information (and the latest release)\n\
can be found at http://geoff.greer.fm/ag\n");
}

void print_version(void) {
    char jit = '-';
    char lzma = '-';
    char zlib = '-';

#ifdef USE_PCRE_JIT
    jit = '+';
#endif
#ifdef HAVE_LZMA_H
    lzma = '+';
#endif
#ifdef HAVE_ZLIB_H
    zlib = '+';
#endif

    printf("ag version %s\n\n", PACKAGE_VERSION);
    printf("Features:\n");
    printf("  %cjit %clzma %czlib\n", jit, lzma, zlib);
}

void init_options(void) {
    char *term = getenv("TERM");

    memset(&opts, 0, sizeof(opts));
    opts.casing = CASE_DEFAULT;
    opts.color = TRUE;
    if (term && !strcmp(term, "dumb")) {
        opts.color = FALSE;
    }
    opts.color_win_ansi = FALSE;
    opts.max_matches_per_file = 0;
    opts.max_search_depth = DEFAULT_MAX_SEARCH_DEPTH;
#if defined(__APPLE__) || defined(__MACH__)
    /* mamp() is slower than normal read() on macos. default to off */
    opts.mmap = FALSE;
#else
    opts.mmap = TRUE;
#endif
    opts.multiline = TRUE;
    opts.width = 0;
    opts.path_sep = '\n';
    opts.print_break = TRUE;
    opts.print_path = PATH_PRINT_DEFAULT;
    opts.print_all_paths = FALSE;
    opts.print_line_numbers = TRUE;
    opts.recurse_dirs = TRUE;
    opts.color_path = ag_strdup(color_path);
    opts.color_match = ag_strdup(color_match);
    opts.color_line_number = ag_strdup(color_line_number);
    opts.use_thread_affinity = TRUE;
}

void cleanup_options(void) {
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
    size_t i;
    int path_len = 0;
    int base_path_len = 0;
    int useless = 0;
    int group = 1;
    int help = 0;
    int version = 0;
    int list_file_types = 0;
    int opt_index = 0;
    char *num_end;
    const char *home_dir = getenv("HOME");
    char *ignore_file_path = NULL;
    int accepts_query = 1;
    int needs_query = 1;
    struct stat statbuf;
    int rv;
    size_t lang_count;
    size_t lang_num = 0;
    int has_filetype = 0;

    size_t longopts_len, full_len;
    option_t *longopts;
    char *lang_regex = NULL;
    size_t *ext_index = NULL;
    char *extensions = NULL;
    size_t num_exts = 0;

    init_options();

    option_t base_longopts[] = {
        { "ackmate", no_argument, &opts.ackmate, 1 },
        { "ackmate-dir-filter", required_argument, NULL, 0 },
        { "affinity", no_argument, &opts.use_thread_affinity, 1 },
        { "after", optional_argument, NULL, 'A' },
        { "all-text", no_argument, NULL, 't' },
        { "all-types", no_argument, NULL, 'a' },
        { "before", optional_argument, NULL, 'B' },
        { "break", no_argument, &opts.print_break, 1 },
        { "case-sensitive", no_argument, NULL, 's' },
        { "color", no_argument, &opts.color, 1 },
        { "color-line-number", required_argument, NULL, 0 },
        { "color-match", required_argument, NULL, 0 },
        { "color-path", required_argument, NULL, 0 },
        { "color-win-ansi", no_argument, &opts.color_win_ansi, TRUE },
        { "column", no_argument, &opts.column, 1 },
        { "context", optional_argument, NULL, 'C' },
        { "count", no_argument, NULL, 'c' },
        { "debug", no_argument, NULL, 'D' },
        { "depth", required_argument, NULL, 0 },
        { "filename", no_argument, NULL, 0 },
        { "filename-pattern", required_argument, NULL, 'g' },
        { "file-search-regex", required_argument, NULL, 'G' },
        { "files-with-matches", no_argument, NULL, 'l' },
        { "files-without-matches", no_argument, NULL, 'L' },
        { "fixed-strings", no_argument, NULL, 'F' },
        { "follow", no_argument, &opts.follow_symlinks, 1 },
        { "group", no_argument, &group, 1 },
        { "heading", no_argument, &opts.print_path, PATH_PRINT_TOP },
        { "help", no_argument, NULL, 'h' },
        { "hidden", no_argument, &opts.search_hidden_files, 1 },
        { "ignore", required_argument, NULL, 0 },
        { "ignore-case", no_argument, NULL, 'i' },
        { "ignore-dir", required_argument, NULL, 0 },
        { "invert-match", no_argument, NULL, 'v' },
        /* deprecated for --numbers. Remove eventually. */
        { "line-numbers", no_argument, &opts.print_line_numbers, 2 },
        { "list-file-types", no_argument, &list_file_types, 1 },
        { "literal", no_argument, NULL, 'Q' },
        { "match", no_argument, &useless, 0 },
        { "max-count", required_argument, NULL, 'm' },
        { "mmap", no_argument, &opts.mmap, TRUE },
        { "multiline", no_argument, &opts.multiline, TRUE },
        /* Accept both --no-* and --no* forms for convenience/BC */
        { "no-affinity", no_argument, &opts.use_thread_affinity, 0 },
        { "noaffinity", no_argument, &opts.use_thread_affinity, 0 },
        { "no-break", no_argument, &opts.print_break, 0 },
        { "nobreak", no_argument, &opts.print_break, 0 },
        { "no-color", no_argument, &opts.color, 0 },
        { "nocolor", no_argument, &opts.color, 0 },
        { "no-filename", no_argument, NULL, 0 },
        { "nofilename", no_argument, NULL, 0 },
        { "no-follow", no_argument, &opts.follow_symlinks, 0 },
        { "nofollow", no_argument, &opts.follow_symlinks, 0 },
        { "no-group", no_argument, &group, 0 },
        { "nogroup", no_argument, &group, 0 },
        { "no-heading", no_argument, &opts.print_path, PATH_PRINT_EACH_LINE },
        { "noheading", no_argument, &opts.print_path, PATH_PRINT_EACH_LINE },
        { "no-mmap", no_argument, &opts.mmap, FALSE },
        { "nommap", no_argument, &opts.mmap, FALSE },
        { "no-multiline", no_argument, &opts.multiline, FALSE },
        { "nomultiline", no_argument, &opts.multiline, FALSE },
        { "no-numbers", no_argument, &opts.print_line_numbers, FALSE },
        { "nonumbers", no_argument, &opts.print_line_numbers, FALSE },
        { "no-pager", no_argument, NULL, 0 },
        { "nopager", no_argument, NULL, 0 },
        { "no-recurse", no_argument, NULL, 'n' },
        { "norecurse", no_argument, NULL, 'n' },
        { "null", no_argument, NULL, '0' },
        { "numbers", no_argument, &opts.print_line_numbers, 2 },
        { "only-matching", no_argument, NULL, 'o' },
        { "one-device", no_argument, &opts.one_dev, 1 },
        { "pager", required_argument, NULL, 0 },
        { "parallel", no_argument, &opts.parallel, 1 },
        { "passthrough", no_argument, &opts.passthrough, 1 },
        { "passthru", no_argument, &opts.passthrough, 1 },
        { "path-to-ignore", required_argument, NULL, 'p' },
        { "print0", no_argument, NULL, '0' },
        { "print-all-files", no_argument, NULL, 0 },
        { "print-long-lines", no_argument, &opts.print_long_lines, 1 },
        { "recurse", no_argument, NULL, 'r' },
        { "search-binary", no_argument, &opts.search_binary_files, 1 },
        { "search-files", no_argument, &opts.search_stream, 0 },
        { "search-zip", no_argument, &opts.search_zip_files, 1 },
        { "silent", no_argument, NULL, 0 },
        { "skip-vcs-ignores", no_argument, NULL, 'U' },
        { "smart-case", no_argument, NULL, 'S' },
        { "stats", no_argument, &opts.stats, 1 },
        { "stats-only", no_argument, NULL, 0 },
        { "unrestricted", no_argument, NULL, 'u' },
        { "version", no_argument, &version, 1 },
        { "vimgrep", no_argument, &opts.vimgrep, 1 },
        { "width", required_argument, NULL, 'W' },
        { "word-regexp", no_argument, NULL, 'w' },
        { "workers", required_argument, NULL, 0 },
    };

    lang_count = get_lang_count();
    longopts_len = (sizeof(base_longopts) / sizeof(option_t));
    full_len = (longopts_len + lang_count + 1);
    longopts = ag_malloc(full_len * sizeof(option_t));
    memcpy(longopts, base_longopts, sizeof(base_longopts));
    ext_index = (size_t *)ag_malloc(sizeof(size_t) * lang_count);
    memset(ext_index, 0, sizeof(size_t) * lang_count);

    for (i = 0; i < lang_count; i++) {
        option_t opt = { langs[i].name, no_argument, NULL, 0 };
        longopts[i + longopts_len] = opt;
    }
    longopts[full_len - 1] = (option_t){ NULL, 0, NULL, 0 };

    if (argc < 2) {
        usage();
        cleanup_ignore(root_ignores);
        cleanup_options();
        exit(1);
    }

    rv = fstat(fileno(stdin), &statbuf);
    if (rv == 0) {
        if (S_ISFIFO(statbuf.st_mode) || S_ISREG(statbuf.st_mode)) {
            opts.search_stream = 1;
        }
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

    char *file_search_regex = NULL;
    while ((ch = getopt_long(argc, argv, "A:aB:C:cDG:g:FfHhiLlm:nop:QRrSsvVtuUwW:z0", longopts, &opt_index)) != -1) {
        switch (ch) {
            case 'A':
                if (optarg) {
                    opts.after = strtol(optarg, &num_end, 10);
                    if (num_end == optarg || *num_end != '\0' || errno == ERANGE) {
                        /* This arg must be the search string instead of the after length */
                        optind--;
                        opts.after = DEFAULT_AFTER_LEN;
                    }
                } else {
                    opts.after = DEFAULT_AFTER_LEN;
                }
                break;
            case 'a':
                opts.search_all_files = 1;
                opts.search_binary_files = 1;
                break;
            case 'B':
                if (optarg) {
                    opts.before = strtol(optarg, &num_end, 10);
                    if (num_end == optarg || *num_end != '\0' || errno == ERANGE) {
                        /* This arg must be the search string instead of the before length */
                        optind--;
                        opts.before = DEFAULT_BEFORE_LEN;
                    }
                } else {
                    opts.before = DEFAULT_BEFORE_LEN;
                }
                break;
            case 'C':
                if (optarg) {
                    opts.context = strtol(optarg, &num_end, 10);
                    if (num_end == optarg || *num_end != '\0' || errno == ERANGE) {
                        /* This arg must be the search string instead of the context length */
                        optind--;
                        opts.context = DEFAULT_CONTEXT_LEN;
                    }
                } else {
                    opts.context = DEFAULT_CONTEXT_LEN;
                }
                break;
            case 'c':
                opts.print_count = 1;
                opts.print_filename_only = 1;
                break;
            case 'D':
                set_log_level(LOG_LEVEL_DEBUG);
                break;
            case 'f':
                opts.follow_symlinks = 1;
                break;
            case 'g':
                needs_query = accepts_query = 0;
                opts.match_files = 1;
            /* fall through */
            case 'G':
                if (file_search_regex) {
                    log_err("File search regex (-g or -G) already specified.");
                    usage();
                    exit(1);
                }
                file_search_regex = ag_strdup(optarg);
                break;
            case 'H':
                opts.print_path = PATH_PRINT_TOP;
                break;
            case 'h':
                help = 1;
                break;
            case 'i':
                opts.casing = CASE_INSENSITIVE;
                break;
            case 'L':
                opts.print_nonmatching_files = 1;
                opts.print_path = PATH_PRINT_TOP;
                break;
            case 'l':
                needs_query = 0;
                opts.print_filename_only = 1;
                opts.print_path = PATH_PRINT_TOP;
                break;
            case 'm':
                opts.max_matches_per_file = atoi(optarg);
                break;
            case 'n':
                opts.recurse_dirs = 0;
                break;
            case 'p':
                opts.path_to_ignore = TRUE;
                load_ignore_patterns(root_ignores, optarg);
                break;
            case 'o':
                opts.only_matching = 1;
                break;
            case 'F':
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
                /* Color highlighting doesn't make sense when inverting matches */
                opts.color = 0;
                break;
            case 'V':
                version = 1;
                break;
            case 'w':
                opts.word_regexp = 1;
                break;
            case 'W':
                opts.width = strtol(optarg, &num_end, 10);
                if (num_end == optarg || *num_end != '\0' || errno == ERANGE) {
                    die("Invalid width\n");
                }
                break;
            case 'z':
                opts.search_zip_files = 1;
                break;
            case '0':
                opts.path_sep = '\0';
                break;
            case 0: /* Long option */
                if (strcmp(longopts[opt_index].name, "ackmate-dir-filter") == 0) {
                    compile_study(&opts.ackmate_dir_filter, &opts.ackmate_dir_filter_extra, optarg, 0, 0);
                    break;
                } else if (strcmp(longopts[opt_index].name, "depth") == 0) {
                    opts.max_search_depth = atoi(optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "filename") == 0) {
                    opts.print_path = PATH_PRINT_DEFAULT;
                    opts.print_line_numbers = TRUE;
                    break;
                } else if (strcmp(longopts[opt_index].name, "ignore-dir") == 0) {
                    add_ignore_pattern(root_ignores, optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "ignore") == 0) {
                    add_ignore_pattern(root_ignores, optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "no-filename") == 0 ||
                           strcmp(longopts[opt_index].name, "nofilename") == 0) {
                    opts.print_path = PATH_PRINT_NOTHING;
                    opts.print_line_numbers = FALSE;
                    break;
                } else if (strcmp(longopts[opt_index].name, "no-pager") == 0 ||
                           strcmp(longopts[opt_index].name, "nopager") == 0) {
                    out_fd = stdout;
                    opts.pager = NULL;
                    break;
                } else if (strcmp(longopts[opt_index].name, "pager") == 0) {
                    opts.pager = optarg;
                    break;
                } else if (strcmp(longopts[opt_index].name, "print-all-files") == 0) {
                    opts.print_all_paths = TRUE;
                    break;
                } else if (strcmp(longopts[opt_index].name, "workers") == 0) {
                    opts.workers = atoi(optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-line-number") == 0) {
                    free(opts.color_line_number);
                    ag_asprintf(&opts.color_line_number, "\033[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-match") == 0) {
                    free(opts.color_match);
                    ag_asprintf(&opts.color_match, "\033[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "color-path") == 0) {
                    free(opts.color_path);
                    ag_asprintf(&opts.color_path, "\033[%sm", optarg);
                    break;
                } else if (strcmp(longopts[opt_index].name, "silent") == 0) {
                    set_log_level(LOG_LEVEL_NONE);
                    break;
                } else if (strcmp(longopts[opt_index].name, "stats-only") == 0) {
                    opts.print_filename_only = 1;
                    opts.print_path = PATH_PRINT_NOTHING;
                    opts.stats = 1;
                    break;
                }

                /* Continue to usage if we don't recognize the option */
                if (longopts[opt_index].flag != 0) {
                    break;
                }

                for (i = 0; i < lang_count; i++) {
                    if (strcmp(longopts[opt_index].name, langs[i].name) == 0) {
                        has_filetype = 1;
                        ext_index[lang_num++] = i;
                        break;
                    }
                }
                if (i != lang_count) {
                    break;
                }

                log_err("option %s does not take a value", longopts[opt_index].name);
            /* fall through */
            default:
                usage();
                exit(1);
        }
    }

    if (opts.casing == CASE_DEFAULT) {
        opts.casing = CASE_SMART;
    }

    if (file_search_regex) {
        int pcre_opts = 0;
        if (opts.casing == CASE_INSENSITIVE || (opts.casing == CASE_SMART && is_lowercase(file_search_regex))) {
            pcre_opts |= PCRE_CASELESS;
        }
        if (opts.word_regexp) {
            char *old_file_search_regex = file_search_regex;
            ag_asprintf(&file_search_regex, "\\b%s\\b", file_search_regex);
            free(old_file_search_regex);
        }
        compile_study(&opts.file_search_regex, &opts.file_search_regex_extra, file_search_regex, pcre_opts, 0);
        free(file_search_regex);
    }

    if (has_filetype) {
        num_exts = combine_file_extensions(ext_index, lang_num, &extensions);
        lang_regex = make_lang_regex(extensions, num_exts);
        compile_study(&opts.file_search_regex, &opts.file_search_regex_extra, lang_regex, 0, 0);
    }

    if (extensions) {
        free(extensions);
    }
    free(ext_index);
    if (lang_regex) {
        free(lang_regex);
    }
    free(longopts);

    argc -= optind;
    argv += optind;

    if (opts.pager) {
        out_fd = popen(opts.pager, "w");
        if (!out_fd) {
            perror("Failed to run pager");
            exit(1);
        }
    }

#ifdef HAVE_PLEDGE
    if (opts.skip_vcs_ignores) {
        if (pledge("stdio rpath proc", NULL) == -1) {
            die("pledge: %s", strerror(errno));
        }
    }
#endif

    if (help) {
        usage();
        exit(0);
    }

    if (version) {
        print_version();
        exit(0);
    }

    if (list_file_types) {
        size_t lang_index;
        printf("The following file types are supported:\n");
        for (lang_index = 0; lang_index < lang_count; lang_index++) {
            printf("  --%s\n    ", langs[lang_index].name);
            int j;
            for (j = 0; j < MAX_EXTENSIONS && langs[lang_index].extensions[j]; j++) {
                printf("  .%s", langs[lang_index].extensions[j]);
            }
            printf("\n\n");
        }
        exit(0);
    }

    if (needs_query && argc == 0) {
        log_err("What do you want to search for?");
        exit(1);
    }

    if (home_dir && !opts.search_all_files) {
        log_debug("Found user's home dir: %s", home_dir);
        ag_asprintf(&ignore_file_path, "%s/.agignore", home_dir);
        load_ignore_patterns(root_ignores, ignore_file_path);
        free(ignore_file_path);
    }

    if (!opts.skip_vcs_ignores) {
        FILE *gitconfig_file = NULL;
        size_t buf_len = 0;
        char *gitconfig_res = NULL;

#ifdef _WIN32
        gitconfig_file = popen("git config -z --path --get core.excludesfile 2>NUL", "r");
#else
        gitconfig_file = popen("git config -z --path --get core.excludesfile 2>/dev/null", "r");
#endif
        if (gitconfig_file != NULL) {
            do {
                gitconfig_res = ag_realloc(gitconfig_res, buf_len + 65);
                buf_len += fread(gitconfig_res + buf_len, 1, 64, gitconfig_file);
            } while (!feof(gitconfig_file) && buf_len > 0 && buf_len % 64 == 0);
            gitconfig_res[buf_len] = '\0';
            if (buf_len == 0) {
                free(gitconfig_res);
                const char *config_home = getenv("XDG_CONFIG_HOME");
                if (config_home) {
                    ag_asprintf(&gitconfig_res, "%s/%s", config_home, "git/ignore");
                } else if (home_dir) {
                    ag_asprintf(&gitconfig_res, "%s/%s", home_dir, ".config/git/ignore");
                } else {
                    gitconfig_res = ag_strdup("");
                }
            }
            log_debug("global core.excludesfile: %s", gitconfig_res);
            load_ignore_patterns(root_ignores, gitconfig_res);
            free(gitconfig_res);
            pclose(gitconfig_file);
        }
    }

#ifdef HAVE_PLEDGE
    if (pledge("stdio rpath proc", NULL) == -1) {
        die("pledge: %s", strerror(errno));
    }
#endif

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

    if (opts.vimgrep) {
        opts.color = 0;
        opts.print_break = 0;
        group = 1;
        opts.search_stream = 0;
        opts.print_path = PATH_PRINT_NOTHING;
    }

    if (opts.parallel) {
        opts.search_stream = 0;
    }

    if (!(opts.print_path != PATH_PRINT_DEFAULT || opts.print_break == 0)) {
        if (group) {
            opts.print_break = 1;
        } else {
            opts.print_path = PATH_PRINT_DEFAULT_EACH_LINE;
            opts.print_break = 0;
        }
    }

    if (opts.search_stream) {
        opts.print_break = 0;
        opts.print_path = PATH_PRINT_NOTHING;
        if (opts.print_line_numbers != 2) {
            opts.print_line_numbers = 0;
        }
    }

    if (accepts_query && argc > 0) {
        if (!needs_query && strlen(argv[0]) == 0) {
            // use default query
            opts.query = ag_strdup(".");
        } else {
            // use the provided query
            opts.query = ag_strdup(argv[0]);
        }
        argc--;
        argv++;
    } else if (!needs_query) {
        // use default query
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
    char *base_path = NULL;
#ifdef PATH_MAX
    char *tmp = NULL;
#endif
    opts.paths_len = argc;
    if (argc > 0) {
        *paths = ag_calloc(sizeof(char *), argc + 1);
        *base_paths = ag_calloc(sizeof(char *), argc + 1);
        for (i = 0; i < (size_t)argc; i++) {
            path = ag_strdup(argv[i]);
            path_len = strlen(path);
            /* kill trailing slash */
            if (path_len > 1 && path[path_len - 1] == '/') {
                path[path_len - 1] = '\0';
            }
            (*paths)[i] = path;
#ifdef PATH_MAX
            tmp = ag_malloc(PATH_MAX);
            base_path = realpath(path, tmp);
#else
            base_path = realpath(path, NULL);
#endif
            if (base_path) {
                base_path_len = strlen(base_path);
                /* add trailing slash */
                if (base_path_len > 1 && base_path[base_path_len - 1] != '/') {
                    base_path = ag_realloc(base_path, base_path_len + 2);
                    base_path[base_path_len] = '/';
                    base_path[base_path_len + 1] = '\0';
                }
            }
            (*base_paths)[i] = base_path;
        }
        /* Make sure we search these paths instead of stdin. */
        opts.search_stream = 0;
    } else {
        path = ag_strdup(".");
        *paths = ag_malloc(sizeof(char *) * 2);
        *base_paths = ag_malloc(sizeof(char *) * 2);
        (*paths)[0] = path;
#ifdef PATH_MAX
        tmp = ag_malloc(PATH_MAX);
        (*base_paths)[0] = realpath(path, tmp);
#else
        (*base_paths)[0] = realpath(path, NULL);
#endif
        i = 1;
    }
    (*paths)[i] = NULL;
    (*base_paths)[i] = NULL;

#ifdef _WIN32
    windows_use_ansi(opts.color_win_ansi);
#endif
}
