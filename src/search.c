#include "search.h"

void search_buf(const pcre *re, const pcre_extra *re_extra,
                const char *buf, const off_t buf_len,
                const char *dir_full_path) {
    int binary = 0;
    long buf_offset = 0;
    match matches[opts.max_matches_per_file];
    long matches_len = 0;
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
    int open_flags = O_RDONLY;
    int mmap_flags = MAP_SHARED;
    FILE *pipe = NULL;

    fd = open(file_full_path, open_flags);
    if (fd < 0) {
        log_err("Error opening file %s: %s Skipping...", file_full_path, strerror(errno));
        goto cleanup;
    }

    rv = fstat(fd, &statbuf);
    if (rv != 0) {
        log_err("Error fstat()ing file %s: %s Skipping...", file_full_path, strerror(errno));
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

        buf = mmap(0, f_len, PROT_READ, mmap_flags, fd, 0);
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
    size_t path_len = 0;
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
            path_len = (size_t)(strlen(path) + strlen(dir->d_name) + 2); /* 2 for slash and null char */
            dir_full_path = malloc(path_len);
            strlcpy(dir_full_path, path, path_len);
            strlcat(dir_full_path, "/", path_len);
            strlcat(dir_full_path, dir->d_name, path_len);
            if (strcmp(SVN_DIR, dir->d_name) == 0) {
                log_debug("svn ignore pattern matched for %s", dir_full_path);
                load_svn_ignore_patterns(dir_full_path, strlen(dir_full_path));
            }
            else {
                load_ignore_patterns(dir_full_path);
            }
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
        if ( ! filepath_filter(dir_full_path)) {
            goto cleanup;
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
