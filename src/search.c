#include "search.h"

void search_buf(const pcre *re, const pcre_extra *re_extra,
                const char *buf, const int buf_len,
                const char *dir_full_path) {
    int binary = 0;
    int buf_offset = 0;
    match matches[opts.max_matches_per_file];
    int matches_len = 0;
    int offset_vector[opts.max_matches_per_file * 3]; /* TODO */
    int rc = 0;

    if (is_binary((void*)buf, buf_len)) { /* Who needs duck typing when you have void cast? :) */
        if (opts.search_binary_files) {
            binary = 1;
        }
        else {
            log_debug("File %s is binary. Skipping...", dir_full_path);
            return;
        }
    }

    if (opts.literal) {
        const char *match_ptr = buf;
        char *(*ag_strncmp_fp)(const char*, const char*, const size_t, const size_t, const size_t[]) = &boyer_moore_strnstr;

        if (opts.casing == CASE_INSENSITIVE) {
            ag_strncmp_fp = &boyer_moore_strncasestr;
        }
        while (buf_offset < buf_len) {
            match_ptr = ag_strncmp_fp(match_ptr, opts.query, buf_len - buf_offset, opts.query_len, skip_lookup);
            if (match_ptr == NULL) {
                break;
            }
            matches[matches_len].start = match_ptr - buf;
            matches[matches_len].end = matches[matches_len].start + opts.query_len;
            buf_offset = matches[matches_len].end;
            log_debug("Match found. File %s, offset %i bytes.", dir_full_path, matches[matches_len].start);
            matches_len++;
            match_ptr += opts.query_len;
            /* Don't segfault. TODO: realloc this array */
            if (matches_len >= opts.max_matches_per_file) {
                log_err("Too many matches in %s. Skipping the rest of this file.", dir_full_path);
                break;
            }
        }
    }
    else {
        /* In my profiling, most of the execution time is spent in this pcre_exec */
        while (buf_offset < buf_len &&
              (rc = pcre_exec(re, re_extra, buf, buf_len, buf_offset, 0, offset_vector, 3)) >= 0) {
            log_debug("Regex match found. File %s, offset %i bytes.", dir_full_path, offset_vector[0]);
            buf_offset = offset_vector[1];
            matches[matches_len].start = offset_vector[0];
            matches[matches_len].end = offset_vector[1];
            matches_len++;
            /* Don't segfault. TODO: realloc this array */
            if (matches_len >= opts.max_matches_per_file) {
                log_err("Too many matches in %s. Skipping the rest of this file.", dir_full_path);
                break;
            }
        }
    }

    if (opts.invert_match) {
        matches_len = invert_matches(matches, matches_len, buf_len);
    }

    if (opts.stats) {
        stats.total_bytes += buf_len;
        stats.total_files++;
        stats.total_matches += matches_len;
    }

    if (matches_len > 0) {
        if (opts.print_filename_only) {
            print_path(dir_full_path, '\n');
        }
        else {
            if (binary) {
                print_binary_file_matches(dir_full_path);
            }
            else {
                print_file_matches(dir_full_path, buf, buf_len, matches, matches_len);
            }
        }
    }
    else {
        log_debug("No match in %s", dir_full_path);
    }
}

/* TODO: this will only match single lines. multi-line regexes silently don't match */
void search_stdin(const pcre *re, const pcre_extra *re_extra) {
    char *line = NULL;
    ssize_t line_length = 0;
    size_t line_cap = 0;

    while ((line_length = getline(&line, &line_cap, stdin)) > 0) {
        search_buf(re, re_extra, line, line_length, "");
    }

    free(line);
}

void search_file(const pcre *re, const pcre_extra *re_extra, const char *file_full_path) {
    int fd = -1;
    off_t f_len = 0;
    char *buf = NULL;
    struct stat statbuf;
    int rv = 0;

    fd = open(file_full_path, O_RDONLY);
    if (fd < 0) {
        log_err("Error opening file %s: %s Skipping...", file_full_path, strerror(errno));
        goto cleanup;
    }

    rv = fstat(fd, &statbuf);
    if (rv != 0) {
        log_err("Error fstat()ing file %s. Skipping...", file_full_path);
        goto cleanup;
    }

    f_len = statbuf.st_size;

    if (f_len == 0) {
        log_debug("File %s is empty, skipping.", file_full_path);
        goto cleanup;
    }

    buf = mmap(0, f_len, PROT_READ, MAP_SHARED, fd, 0);
    if (buf == MAP_FAILED) {
        log_err("File %s failed to load: %s.", file_full_path, strerror(errno));
        goto cleanup;
    }

    search_buf(re, re_extra, buf, (int)f_len, file_full_path);

    cleanup:
    if (fd != -1) {
        munmap(buf, f_len);
        close(fd);
    }
}

/* TODO: append matches to some data structure instead of just printing them out
 * then there can be sweet summaries of matches/files scanned/time/etc
 */
void search_dir(const pcre *re, const pcre_extra *re_extra, const char* path, const int depth) {
    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    int fd = -1;
    off_t f_len = 0;
    char *buf = NULL;
    char *dir_full_path = NULL;
    size_t path_length = 0;
    int i;

    /* find agignore/gitignore/hgignore/etc files to load ignore patterns from */
#ifdef AG_OS_BSD
    results = scandir(path, &dir_list, &ignorefile_filter, &alphasort);
#else
    results = scandir(path, &dir_list, (int (*)(const struct dirent *))&ignorefile_filter, &alphasort);
#endif
    if (results > 0) {
        for (i = 0; i < results; i++) {
            dir = dir_list[i];
            path_length = (size_t)(strlen(path) + strlen(dir->d_name) + 2); /* 2 for slash and null char */
            dir_full_path = malloc(path_length);
            strlcpy(dir_full_path, path, path_length);
            strlcat(dir_full_path, "/", path_length);
            strlcat(dir_full_path, dir->d_name, path_length);
            load_ignore_patterns(dir_full_path);
            free(dir);
            dir = NULL;
            free(dir_full_path);
            dir_full_path = NULL;
        }
    }
    free(dir_list);
    dir_list = NULL;

#ifdef AG_OS_BSD
    results = scandir(path, &dir_list, &filename_filter, &alphasort);
#else
    results = scandir(path, &dir_list, (int (*)(const struct dirent *))&filename_filter, &alphasort);
#endif
    if (results == 0)
    {
        log_debug("No results found in directory %s", path);
        free(dir_list);
        dir_list = NULL;
        return;
    }
    else if (results == -1) {
        if (errno == ENOTDIR) {
            /* Not a directory. Probably a file. */
            /* If we're only searching one file, don't print the filename header at the top. */
            opts.print_heading = depth == 0 ? -1 : opts.print_heading;
            search_file(re, re_extra, path);
            return;
        }
        else {
            log_err("Error opening directory %s: %s", path, strerror(errno));
            return;
        }
    }

    int offset_vector[3];
    int rc = 0;

    for (i=0; i<results; i++) {
        dir = dir_list[i];
        /* TODO: this is copy-pasted from about 30 lines above */
        path_length = (size_t)(strlen(path) + strlen(dir->d_name) + 2); /* 2 for slash and null char */
        dir_full_path = malloc(path_length);
        strlcpy(dir_full_path, path, path_length);
        strlcat(dir_full_path, "/", path_length);
        strlcat(dir_full_path, dir->d_name, path_length);

        log_debug("dir %s type %i", dir_full_path, dir->d_type);

        if (opts.file_search_regex) {
            rc = pcre_exec(opts.file_search_regex, NULL, dir_full_path, strlen(dir_full_path),
                           0, 0, offset_vector, 3);
            if (rc < 0) { /* no match */
                log_debug("Skipping %s due to file_search_regex.", dir_full_path);
                goto cleanup;
            }
        }

        /* TODO: scan files in current dir before going deeper */
        if (dir->d_type == DT_DIR) {
            if (opts.recurse_dirs) {
                if (depth < opts.max_search_depth) {
                    log_debug("Searching dir %s", dir_full_path);
                    search_dir(re, re_extra, dir_full_path, depth + 1);
                }
                else {
                    log_err("Skipping %s. Use the --depth option to search deeper.", dir_full_path);
                }
            }
            goto cleanup;
        }

        search_file(re, re_extra, dir_full_path);

        cleanup:
        if (fd != -1) {
            munmap(buf, f_len);
            close(fd);
            fd = -1;
        }

        free(dir);
        dir = NULL;
        free(dir_full_path);
        dir_full_path = NULL;
    }

    free(dir_list);
    dir_list = NULL;
    return;
}
