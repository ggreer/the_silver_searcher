#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"

int first_file_match = 1;

const char *colors_reset = "\e[0m\e[K";
const char *colors_path = "\e[1;32m";   /* bold green */
const char *colors_match = "\e[30;43m"; /* black with yellow background */
const char *colors_line_number = "\e[1;33m"; /* yellow with black background */

void print_path(const char* path) {
    log_debug("printing path");

    if (opts.ackmate) {
        printf(":%s", path);
    }
    else {
        if (opts.color) {
            printf("%s%s%s", colors_path, path, colors_reset);
        }
        else {
            printf("%s", path);
        }
    }
}

void print_binary_file_matches(const char* path) {
    print_file_separator();
    printf("Binary file %s matches.\n", path);
}

/* TODO: doesn't work for matches across lines */
void print_file_matches(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len) {
    int line = 1;
    int column = 0;
    char **context_prev_lines = NULL;
    int prev_line = 0;
    int last_prev_line = 0;
    int prev_line_offset = 0;
    int cur_match = 0;
    int in_a_match = 0;
    /* TODO the line below contains a terrible hack */
    int lines_since_last_match = 1000000; /* if I initialize this to INT_MAX it'll overflow */
    int last_printed_match = 0;
    char sep = '-';
    int i, j;

    if (opts.ackmate) {
        sep = ':';
    }

    print_file_separator();

    if (opts.print_heading) {
        print_path(path);
        printf("\n");
    }

    context_prev_lines = malloc(sizeof(char*) * (opts.before + 1));

    for (i = 0; i < opts.before; i++) {
        context_prev_lines[i] = NULL;
    }

    for (i = 0; i < buf_len && (cur_match < matches_len || lines_since_last_match <= opts.after); i++) {
        if (cur_match < matches_len && i == matches[cur_match].start) {
            /* We found the start of a match */
            in_a_match = 1;

            if (cur_match > 0 && opts.context && lines_since_last_match > (opts.before + opts.after + 1)) {
                printf("--\n");
            }

            if (lines_since_last_match > 0) {
                if (opts.before > 0) {
                    /* TODO: this is buggy as hell */
                    /* TODO: yeah, it totally is */
                    /* print the previous line(s) */
                    int lines_to_print = lines_since_last_match - (opts.before + opts.after + 1);
                    if (lines_to_print < 0) {
                        lines_to_print = opts.before;
                    }
                    if (lines_to_print > opts.before) {
                        lines_to_print = 0;
                    }

                    for (j = lines_to_print; j < opts.before; j++) {
                        prev_line = (last_prev_line + j) % opts.before;
                        if (context_prev_lines[prev_line] != NULL) {
                            if (opts.print_heading == 0) {
                                print_path(path);
                                printf(":");
                            }
                            print_line_number(line - (opts.before - j), sep);
                            printf("%s\n", context_prev_lines[prev_line]);
                        }
                    }
                }

                if (opts.ackmate == 0) {
                    if (opts.print_heading == 0) {
                        print_path(path);
                        printf(":");
                    }
                    print_line_number(line, ':');

                    if (opts.column) {
                        printf("%i:", column + 1);
                    }
                    /* print up to current char */
                    for (j = prev_line_offset; j < i; j++) {
                        putchar(buf[j]);
                    }
                }
            }

            lines_since_last_match = 0;
            if (opts.color) {
                printf("%s", colors_match);
            }
        }

        if (cur_match < matches_len && i == matches[cur_match].end) {
            /* We found the end of a match. */
            in_a_match = 0;
            cur_match++;
            if (opts.color) {
                printf("%s", colors_reset);
            }
        }

        if ((in_a_match || lines_since_last_match == 0) && opts.ackmate == 0) {
            putchar(buf[i]);
        }

        column++;

        /* We found the end of a line. */
        if (buf[i] == '\n') {
            if (opts.before > 0) {
                if (context_prev_lines[last_prev_line] != NULL) {
                    free(context_prev_lines[last_prev_line]);
                }
                /* We just incremented column so it will always be at least 1.
                 * We don't want to strcpy the \n */
                context_prev_lines[last_prev_line] = strndup(&buf[prev_line_offset], i - prev_line_offset);
                last_prev_line = (last_prev_line + 1) % opts.before;
            }
        }

        if (buf[i] == '\n' || i == buf_len - 1) {
            if (lines_since_last_match == 0) {
                if (opts.ackmate) {
                    /* print headers for ackmate to parse */
                    print_line_number(line, ';');
                    while (last_printed_match < cur_match) {
                        printf("%i %i", (matches[last_printed_match].start - prev_line_offset), (matches[last_printed_match].end - matches[last_printed_match].start));
                        if (last_printed_match == cur_match - 1) {
                            putchar(':');
                        }
                        else {
                            putchar(',');
                        }
                        last_printed_match++;
                    }
                    /* print up to current char */
                    for (j = prev_line_offset; j < i; j++) {
                        putchar(buf[j]);
                    }
                    putchar('\n');
                }
            }
            else if (lines_since_last_match <= opts.after) {
                /* print context after matching line */
                if (opts.print_heading == 0) {
                    print_path(path);
                    printf(":");
                }
                print_line_number(line, sep);

                for (j = prev_line_offset; j < i; j++) {
                    putchar(buf[j]);
                }
                putchar('\n');
            }

            prev_line_offset = i + 1; /* skip the newline */
            line++;
            column = 0;
            lines_since_last_match++;
        }
    }

    free(context_prev_lines);
}

void print_line_number(int line, char sep) {
    log_debug("printing line number");
    if (opts.ackmate) {
        printf("%i%c", line, sep);
    }
    else {
        if (opts.color) {
            printf("%s%i%s%c", colors_line_number, line, colors_reset, sep);
        }
        else {
            printf("%i%c", line, sep);
        }
    }
}

void print_file_separator() {
    log_debug("printing file separator");
    if (first_file_match == 0 && opts.print_break) {
        printf("\n");
    }
    first_file_match = 0;
}
