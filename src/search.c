#include "search.h"

void search_buf(const pcre *re, const pcre_extra *re_extra,
                const char *buf, const int buf_len,
                const char *dir_full_path) {
    int binary = 0;
    int buf_offset = 0;
    match *matches = NULL;
    size_t matches_size = 100;
    int matches_len = 0;
    int *offset_vector = NULL;
    int rc = 0;

    /* Who needs duck typing when you have void cast? :) */
    if (is_binary((void*)buf, buf_len)) {
        if (opts.search_binary_files) {
            binary = 1;
        }
        else {
            log_debug("File %s is binary. Skipping...", dir_full_path);
            return;
        }
    }

    matches = malloc(sizeof(match) * matches_size);
    offset_vector = malloc(sizeof(int) * matches_size * 3);

    if (opts.literal) {
        const char *match_ptr = buf;
        strncmp_fp ag_strnstr_fp = get_strstr(opts);

        while (buf_offset < buf_len) {
            match_ptr = ag_strnstr_fp(match_ptr, opts.query, buf_len - buf_offset, opts.query_len, skip_lookup);
            if (match_ptr == NULL) {
                break;
            }

            if (opts.word_regexp) {
                int word_start = FALSE;
                int word_end = FALSE;
                const char *start = match_ptr;
                const char *end = match_ptr + opts.query_len;

                if (start == buf) {
                    /* Start of string. */
                    word_start = TRUE;
                }
                else if (is_whitespace(*(start - 1))) {
                    word_start = TRUE;
                }

                if (*end == '\0') {
                    /* End of string. */
                    word_end = TRUE;
                }
                else if (is_whitespace(*end)) {
                    word_end = TRUE;
                }
                /* Skip if we're not a word. */
                if (!(word_start && word_end)) {
                    match_ptr += opts.query_len;
                    buf_offset = end - buf;
                    continue;
                }
            }

            matches[matches_len].start = match_ptr - buf;
            matches[matches_len].end = matches[matches_len].start + opts.query_len;
            buf_offset = matches[matches_len].end;
            log_debug("Match found. File %s, offset %i bytes.", dir_full_path, matches[matches_len].start);
            matches_len++;
            match_ptr += opts.query_len;
            if (matches_len >= opts.max_matches_per_file) {
                log_err("Too many matches in %s. Skipping the rest of this file.", dir_full_path);
                break;
            }
            else if ((size_t)matches_len >= matches_size) {
                matches_size = matches_size * 2;
                matches = realloc(matches, matches_size);
                log_debug("Too many matches in %s. Reallocating matches to %u.", dir_full_path, matches_size);
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
            /* TODO: copy-pasted from above. FIXME */
            if (matches_len >= opts.max_matches_per_file) {
                log_err("Too many matches in %s. Skipping the rest of this file.", dir_full_path);
                break;
            }
            else if ((size_t)matches_len >= matches_size) {
                matches_size = matches_size * 2;
                matches = realloc(matches, matches_size);
                offset_vector = realloc(offset_vector, sizeof(int) * matches_size * 3);
                log_debug("Too many matches in %s. Reallocating matches to %u.", dir_full_path, matches_size);
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

    free(matches);
    free(offset_vector);
}

void search_stdin(const pcre *re, const pcre_extra *re_extra) {
    search_stream(re, re_extra, stdin, "");
}

/* TODO: this will only match single lines. multi-line regexes silently don't match */
void search_stream(const pcre *re, const pcre_extra *re_extra, FILE *stream, const char *path) {
    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    opts.print_break = 0;
    opts.print_heading = 0;
    opts.print_line_numbers = 0;
    opts.search_stream = 1;

    while ((line_len = getline(&line, &line_cap, stream)) > 0) {
        search_buf(re, re_extra, line, line_len, path);
    }

    free(line);
}

void search_file(const pcre *re, const pcre_extra *re_extra, const char *file_full_path) {
    int fd = -1;
    off_t f_len = 0;
    char *buf = NULL;
    struct stat statbuf;
    int rv = 0;
    FILE *pipe = NULL;

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

    if ((statbuf.st_mode & S_IFMT) == 0) {
        log_err("%s is not a file. Mode %u. Skipping...", file_full_path, statbuf.st_mode);
        goto cleanup;
    }

    if (statbuf.st_mode & S_IFIFO) {
        log_debug("%s is a named pipe. stream searching", file_full_path);
        pipe = fdopen(fd, "r");
        search_stream(re, re_extra, pipe, file_full_path);
        fclose(pipe);
    }
    else {
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
    }

    cleanup:
    if (fd != -1) {
        munmap(buf, f_len);
        close(fd);
    }
}

/* TODO: Append matches to some data structure instead of just printing them out.
 * Then ag can have sweet summaries of matches/files scanned/time/etc.
 */
void search_dir(const pcre *re, const pcre_extra *re_extra, const char* path, const int depth) {
    struct dirent **dir_list = NULL;
    struct dirent *dir = NULL;
    int results = 0;

    int fd = -1;
    off_t f_len = 0;
    char *buf = NULL;
    char *dir_full_path = NULL;
    const char *ignore_file = NULL;
    size_t path_len = 0;
    int i;

    /* find agignore/gitignore/hgignore/etc files to load ignore patterns from */
    for (i = 0; ignore_pattern_files[i] != NULL; i++) {
        ignore_file = ignore_pattern_files[i];
        path_len = (size_t)(strlen(path) + strlen(ignore_file) + 2); /* 2 for slash and null char */
        dir_full_path = malloc(path_len);
        strlcpy(dir_full_path, path, path_len);
        strlcat(dir_full_path, "/", path_len);
        strlcat(dir_full_path, ignore_file, path_len);
        if (strcmp(SVN_DIR, ignore_file) == 0) {
            log_debug("svn ignore pattern matched for %s", dir_full_path);
            load_svn_ignore_patterns(dir_full_path, strlen(dir_full_path));
        }
        else {
            load_ignore_patterns(dir_full_path);
        }
        free(dir_full_path);
        dir_full_path = NULL;
    }

#ifdef SCANDIR_CONST
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
            if (depth == 0 && opts.paths_len == 1) {
                opts.print_heading = -1;
            }
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
    struct stat stDirInfo;

    for (i = 0; i < results; i++) {
        dir = dir_list[i];
        /* TODO: this is copy-pasted from about 30 lines above */
        path_len = (size_t)(strlen(path) + strlen(dir->d_name) + 2); /* 2 for slash and null char */
        dir_full_path = malloc(path_len);
        strlcpy(dir_full_path, path, path_len);
        strlcat(dir_full_path, "/", path_len);
        strlcat(dir_full_path, dir->d_name, path_len);

        /* Some filesystems, e.g. ReiserFS, always return a type DT_UNKNOWN from readdir or scandir. */
        /* Call lstat if we find DT_UNKNOWN to get the information we need. */
        if (dir->d_type == DT_UNKNOWN) {
            if (lstat(dir_full_path, &stDirInfo) != -1) {
                if (S_ISDIR(stDirInfo.st_mode)) {
                    dir->d_type = DT_DIR;
                }
                else if (S_ISLNK(stDirInfo.st_mode)) {
                    dir->d_type = DT_LNK;
                }
            }
            else {
                log_err("lstat() failed on %s", dir_full_path);
                /* If lstat fails we may as well carry on and hope for the best. */
            }

            if (!opts.follow_symlinks && dir->d_type == DT_LNK) {
                log_debug("File %s ignored becaused it's a symlink", dir->d_name);
                goto cleanup;
            }
        }

        /* If a link points to a directory then we need to treat it as a directory. */
        if (dir->d_type == DT_LNK) {
            if (stat(dir_full_path, &stDirInfo) != -1) {
                if (S_ISDIR(stDirInfo.st_mode)) {
                    dir->d_type = DT_DIR;
                }
            }
            else {
                log_err("stat() failed on %s", dir_full_path);
                /* If stat fails we may as well carry on and hope for the best. */
            }
        }

        log_debug("dir %s type %i", dir_full_path, dir->d_type);
        if (!filepath_filter(dir_full_path)) {
            goto cleanup;
        }

        if (dir->d_type != DT_DIR) {
            if (opts.file_search_regex) {
                rc = pcre_exec(opts.file_search_regex, NULL, dir_full_path, strlen(dir_full_path),
                               0, 0, offset_vector, 3);
                if (rc < 0) { /* no match */
                    log_debug("Skipping %s due to file_search_regex.", dir_full_path);
                    goto cleanup;
                }
                else if (opts.match_files) {
                    log_debug("match_files: file_search_regex matched for %s.", dir_full_path);
                    print_path(dir_full_path, '\n');
                    goto cleanup;
                }
            }

            search_file(re, re_extra, dir_full_path);
        }
        else if (opts.recurse_dirs) {
            if (depth < opts.max_search_depth) {
                log_debug("Searching dir %s", dir_full_path);
                search_dir(re, re_extra, dir_full_path, depth + 1);
            }
            else {
                log_err("Skipping %s. Use the --depth option to search deeper.", dir_full_path);
            }
        }

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
