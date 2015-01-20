#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

int first_file_match = 1;

const char *color_reset = "\033[0m\033[K";

void print_path(const char *path, const char sep) {
    path = normalize_path(path);

    if (opts.ackmate) {
        fprintf(out_fd, ":%s%c", path, sep);
    } else if (opts.vimgrep) {
        fprintf(out_fd, "%s%c", path, sep);
    } else {
        if (opts.color) {
            fprintf(out_fd, "%s%s%s%c", opts.color_path, path, color_reset, sep);
        } else {
            fprintf(out_fd, "%s%c", path, sep);
        }
    }
}

void print_path_count(const char *path, const char sep, const size_t count) {
    if (*path) {
        print_path(path, ':');
    }
    if (opts.color) {
        fprintf(out_fd, "%s%lu%s%c", opts.color_line_number, count, color_reset, sep);
    } else {
        fprintf(out_fd, "%lu%c", count, sep);
    }
}

void print_line(const char *buf, size_t buf_pos, size_t prev_line_offset) {
    for (; prev_line_offset <= buf_pos; prev_line_offset++) {
        fputc(buf[prev_line_offset], out_fd);
    }
}

void print_binary_file_matches(const char *path) {
    path = normalize_path(path);
    print_file_separator();
    fprintf(out_fd, "Binary file %s matches.\n", path);
}

void print_file_matches(const char *path, const char *buf, const size_t buf_len, const match_t matches[], const size_t matches_len) {
    size_t line = 1;
    char **context_prev_lines = NULL;
    size_t prev_line = 0;
    size_t last_prev_line = 0;
    size_t prev_line_offset = 0;
    size_t cur_match = 0;
    size_t lines_since_last_match = INT_MAX;
    ssize_t lines_to_print = 0;
    size_t last_printed_match = 0;
    char sep = '-';
    size_t i, j;
    int in_a_match = FALSE;
    int printing_a_match = FALSE;

    if (opts.ackmate || opts.vimgrep) {
        sep = ':';
    }

    print_file_separator();

    if (opts.print_path == PATH_PRINT_DEFAULT) {
        opts.print_path = PATH_PRINT_TOP;
    } else if (opts.print_path == PATH_PRINT_DEFAULT_EACH_LINE) {
        opts.print_path = PATH_PRINT_EACH_LINE;
    }

    if (opts.print_path == PATH_PRINT_TOP) {
        if (opts.print_count) {
            print_path_count(path, opts.path_sep, matches_len);
        } else {
            print_path(path, opts.path_sep);
        }
    }

    context_prev_lines = ag_calloc(sizeof(char *), (opts.before + 1));

    for (i = 0; i <= buf_len && (cur_match < matches_len || lines_since_last_match <= opts.after); i++) {
        if (cur_match < matches_len && i == matches[cur_match].start) {
            in_a_match = TRUE;
            /* We found the start of a match */
            if (cur_match > 0 && opts.context && lines_since_last_match > (opts.before + opts.after + 1)) {
                fprintf(out_fd, "--\n");
            }

            if (lines_since_last_match > 0 && opts.before > 0) {
                /* TODO: better, but still needs work */
                /* print the previous line(s) */
                lines_to_print = lines_since_last_match - (opts.after + 1);
                if (lines_to_print < 0) {
                    lines_to_print = 0;
                } else if ((size_t)lines_to_print > opts.before) {
                    lines_to_print = opts.before;
                }

                for (j = (opts.before - lines_to_print); j < opts.before; j++) {
                    prev_line = (last_prev_line + j) % opts.before;
                    if (context_prev_lines[prev_line] != NULL) {
                        if (opts.print_path == PATH_PRINT_EACH_LINE) {
                            print_path(path, ':');
                        }
                        print_line_number(line - (opts.before - j), sep);
                        fprintf(out_fd, "%s\n", context_prev_lines[prev_line]);
                    }
                }
            }
            lines_since_last_match = 0;
        }

        if (cur_match < matches_len && i == matches[cur_match].end) {
            /* We found the end of a match. */
            cur_match++;
            in_a_match = FALSE;
        }

        /* We found the end of a line. */
        if (buf[i] == '\n' && opts.before > 0) {
            if (context_prev_lines[last_prev_line] != NULL) {
                free(context_prev_lines[last_prev_line]);
            }
            /* We don't want to strcpy the \n */
            context_prev_lines[last_prev_line] = ag_strndup(&buf[prev_line_offset], i - prev_line_offset);
            last_prev_line = (last_prev_line + 1) % opts.before;
        }

        if (buf[i] == '\n' || i == buf_len) {
            if (lines_since_last_match == 0) {
                if (opts.print_path == PATH_PRINT_EACH_LINE && !opts.search_stream) {
                    print_path(path, ':');
                }
                if (opts.ackmate) {
                    /* print headers for ackmate to parse */
                    print_line_number(line, ';');
                    for (; last_printed_match < cur_match; last_printed_match++) {
                        fprintf(out_fd, "%lu %lu",
                                (matches[last_printed_match].start - prev_line_offset),
                                (matches[last_printed_match].end - matches[last_printed_match].start));
                        last_printed_match == cur_match - 1 ? fputc(':', out_fd) : fputc(',', out_fd);
                    }
                    print_line(buf, i, prev_line_offset);
                } else if (opts.vimgrep) {
                    for (; last_printed_match < cur_match; last_printed_match++) {
                        print_path(path, sep);
                        print_line_number(line, sep);
                        print_column_number(matches, last_printed_match, prev_line_offset, sep);
                        print_line(buf, i, prev_line_offset);
                    }
                } else {
                    print_line_number(line, ':');
                    int printed_match = FALSE;
                    if (opts.column) {
                        print_column_number(matches, last_printed_match, prev_line_offset, ':');
                    }

                    if (printing_a_match && opts.color) {
                        fprintf(out_fd, "%s", opts.color_match);
                    }
                    for (j = prev_line_offset; j <= i; j++) {
                        if (last_printed_match < matches_len && j == matches[last_printed_match].end) {
                            if (opts.color) {
                                fprintf(out_fd, "%s", color_reset);
                            }
                            printing_a_match = FALSE;
                            last_printed_match++;
                            printed_match = TRUE;
                            if (opts.only_matching) {
                                fputc('\n', out_fd);
                            }
                        }
                        if (last_printed_match < matches_len && j == matches[last_printed_match].start) {
                            if (opts.only_matching && printed_match) {
                                if (opts.print_path == PATH_PRINT_EACH_LINE) {
                                    print_path(path, ':');
                                }
                                print_line_number(line, ':');
                            }
                            if (opts.color) {
                                fprintf(out_fd, "%s", opts.color_match);
                            }
                            printing_a_match = TRUE;
                        }
                        /* Don't print the null terminator */
                        if (j < buf_len) {
                            /* if only_matching is set, print only matches and newlines */
                            if (!opts.only_matching || printing_a_match) {
                                fputc(buf[j], out_fd);
                            }
                        }
                    }
                    if (printing_a_match && opts.color) {
                        fprintf(out_fd, "%s", color_reset);
                    }
                }
            } else if (lines_since_last_match <= opts.after) {
                /* print context after matching line */
                if (opts.print_path == PATH_PRINT_EACH_LINE) {
                    print_path(path, ':');
                }
                print_line_number(line, sep);

                for (j = prev_line_offset; j < i; j++) {
                    fputc(buf[j], out_fd);
                }
                fputc('\n', out_fd);
            }

            prev_line_offset = i + 1; /* skip the newline */
            line++;
            if (!in_a_match && lines_since_last_match < INT_MAX) {
                lines_since_last_match++;
            }
            /* File doesn't end with a newline. Print one so the output is pretty. */
            if (i == buf_len && buf[i] != '\n' && !opts.search_stream) {
                fputc('\n', out_fd);
            }
        }
    }

    for (i = 0; i < opts.before; i++) {
        if (context_prev_lines[i] != NULL) {
            free(context_prev_lines[i]);
        }
    }
    free(context_prev_lines);
}

void print_line_number(size_t line, const char sep) {
    if (!opts.print_line_numbers) {
        return;
    }
    if (opts.search_stream && opts.stream_line_num) {
        line = opts.stream_line_num;
    }
    if (opts.color) {
        fprintf(out_fd, "%s%lu%s%c", opts.color_line_number, (unsigned long)line, color_reset, sep);
    } else {
        fprintf(out_fd, "%lu%c", (unsigned long)line, sep);
    }
}

void print_column_number(const match_t matches[], size_t last_printed_match,
                         size_t prev_line_offset, const char sep) {
    fprintf(out_fd, "%lu%c",
            (matches[last_printed_match].start - prev_line_offset) + 1,
            sep);
}

void print_file_separator(void) {
    if (first_file_match == 0 && opts.print_break) {
        fprintf(out_fd, "\n");
    }
    first_file_match = 0;
}

const char *normalize_path(const char *path) {
    if (strlen(path) < 3) {
        return path;
    }
    if (path[0] == '.' && path[1] == '/') {
        return path + 2;
    }
    if (path[0] == '/' && path[1] == '/') {
        return path + 1;
    }
    return path;
}
