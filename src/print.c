#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

int first_file_match = 1;

const char *colors_reset = "\e[0m\e[K";
const char *colors_path = "\e[1;32m";   /* bold green */
const char *colors_match = "\e[30;43m"; /* black with yellow background */
const char *colors_line_number = "\e[1;33m"; /* yellow with black background */

void print_path(const char* path, const char sep) {
    log_debug("printing path");

    if (opts.ackmate) {
        fprintf(out_fd, ":%s%c", path, sep);
    } else {
        if (opts.color) {
            fprintf(out_fd, "%s%s%s%c", colors_path, path, colors_reset, sep);
        } else {
            fprintf(out_fd, "%s%c", path, sep);
        }
    }
}

void print_binary_file_matches(const char* path) {
    print_file_separator();
    fprintf(out_fd, "Binary file %s matches.\n", path);
}

void print_file_matches(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len) {
    int line = 1;
    char **context_prev_lines = NULL;
    int prev_line = 0;
    int last_prev_line = 0;
    int prev_line_offset = 0;
    int cur_match = 0;
    /* TODO the line below contains a terrible hack */
    int lines_since_last_match = 1000000; /* if I initialize this to INT_MAX it'll overflow */
    int lines_to_print = 0;
    int last_printed_match = 0;
    char sep = '-';
    int i, j;
    int in_a_match = FALSE;
    int printing_a_match = FALSE;

    if (opts.ackmate) {
        sep = ':';
    }

    print_file_separator();

    if (opts.print_heading == TRUE) {
        print_path(path, '\n');
    }

    context_prev_lines = ag_calloc(sizeof(char*), (opts.before + 1));

    for (i = 0; i <= buf_len && (cur_match < matches_len || lines_since_last_match <= opts.after); i++) {
        if (cur_match < matches_len && i == matches[cur_match].end) {
            /* We found the end of a match. */
            cur_match++;
            in_a_match = FALSE;
        }

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
                } else if (lines_to_print > opts.before) {
                    lines_to_print = opts.before;
                }

                for (j = (opts.before - lines_to_print); j < opts.before; j++) {
                    prev_line = (last_prev_line + j) % opts.before;
                    if (context_prev_lines[prev_line] != NULL) {
                        if (opts.print_heading == 0) {
                            print_path(path, ':');
                        }
                        print_line_number(line - (opts.before - j), sep);
                        fprintf(out_fd, "%s\n", context_prev_lines[prev_line]);
                    }
                }
            }
            lines_since_last_match = 0;
        }

        /* We found the end of a line. */
        if (buf[i] == '\n' && opts.before > 0) {
            if (context_prev_lines[last_prev_line] != NULL) {
                free(context_prev_lines[last_prev_line]);
            }
            /* We don't want to strcpy the \n */
            context_prev_lines[last_prev_line] = strndup(&buf[prev_line_offset], i - prev_line_offset);
            last_prev_line = (last_prev_line + 1) % opts.before;
        }

        if (buf[i] == '\n' || i == buf_len) {
            if (lines_since_last_match == 0) {
                if (opts.print_heading == 0 && !opts.search_stream) {
                    print_path(path, ':');
                }

                if (opts.ackmate) {
                    /* print headers for ackmate to parse */
                    print_line_number(line, ';');
                    for (; last_printed_match < cur_match; last_printed_match++) {
                        fprintf(out_fd, "%i %i",
                              (matches[last_printed_match].start - prev_line_offset),
                              (matches[last_printed_match].end - matches[last_printed_match].start)
                        );
                        last_printed_match == cur_match - 1 ? fputc(':', out_fd) : fputc(',', out_fd);
                    }
                    j = prev_line_offset;
                    /* print up to current char */
                    for (; j <= i; j++) {
                        fputc(buf[j], out_fd);
                    }
                } else {
                    print_line_number(line, ':');
                    if (opts.column) {
                        fprintf(out_fd, "%i:", (matches[last_printed_match].start - prev_line_offset) + 1);
                    }

                    if (printing_a_match && opts.color) {
                        fprintf(out_fd, "%s", colors_match);
                    }
                    for (j = prev_line_offset; j <= i; j++) {
                        if (j == matches[last_printed_match].end) {
                            if (opts.color) {
                                fprintf(out_fd, "%s", colors_reset);
                            }
                            printing_a_match = FALSE;
                            last_printed_match++;
                        }
                        if (j == matches[last_printed_match].start) {
                            if (opts.color) {
                                fprintf(out_fd, "%s", colors_match);
                            }
                            printing_a_match = TRUE;
                        }
                        fputc(buf[j], out_fd);
                    }
                    if (printing_a_match && opts.color) {
                        fprintf(out_fd, "%s", colors_reset);
                    }
                }
            } else if (lines_since_last_match <= opts.after) {
                /* print context after matching line */
                if (opts.print_heading == 0) {
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
            if (!in_a_match) {
                lines_since_last_match++;
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

void print_line_number(const int line, const char sep) {
    if (!opts.print_line_numbers) {
        return;
    }
    log_debug("printing line number");

    if (opts.color) {
        fprintf(out_fd, "%s%i%s%c", colors_line_number, line, colors_reset, sep);
    } else {
        fprintf(out_fd, "%i%c", line, sep);
    }
}

void print_file_separator() {
    if (first_file_match == 0 && opts.print_break) {
        log_debug("printing file separator");
        fprintf(out_fd, "\n");
    }
    first_file_match = 0;
}
