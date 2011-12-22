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
const char *colors_path = "\e[1;32m"; // bold green
const char *colors_match = "\e[30;43m"; // black with yellow background

void print_path(const char* path) {
    if (opts.ackmate) {
        printf(":%s\n", path);
    }
    else {
        if (opts.color) {
            printf("%s%s%s\n", colors_path, path, colors_reset);
        }
        else {
            printf("%s\n", path);
        }
    }
}

void print_file_matches(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len) {
    int line = 1;
    int column = 0;
    int prev_line_offset = 0;
    int cur_match = 0;
    int in_a_match = 0;
    int lines_since_last_match = 1000000; // super lame, I know
    int last_printed_match = 0;

    if (first_file_match == 0 && opts.ackmate == 0) {
        printf("\n");
    }
    first_file_match = 0;

    print_path(path);

    for (int i = 0; i < buf_len && (cur_match < matches_len || lines_since_last_match == 0); i++) {
        if (i == matches[cur_match].start) {
            in_a_match = 1;

            if (lines_since_last_match > 0) {
                if (opts.ackmate == 0) {
                    printf("%i:", line);
                    // print up to current char
                    for (int j = prev_line_offset; j < i; j++) {
                        putchar(buf[j]);
                    }
                }
            }

            lines_since_last_match = 0;
            if (opts.color) {
                printf("%s", colors_match);
            }
        }

        if (i == matches[cur_match].end) {
            // We found the end of a match.
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

        if (buf[i] == '\n' || i == buf_len - 1) {
            if (opts.ackmate && lines_since_last_match == 0) {
                // print headers for ackmate to parse
                printf("%i;", line);
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
                // print up to current char
                for (int j = prev_line_offset; j < i; j++) {
                    putchar(buf[j]);
                }
                putchar('\n');
            }

            prev_line_offset = i + 1; // skip the newline
            line++;
            column = 0;
            lines_since_last_match++;
        }
    }
}

/*
  Example output of ackmate's ack:
  ./ackmate_ack --ackmate --literal --context --nofollow --ignore-case --match "test" ../
  :../Resources/ackmate_ack
  756:
  757:That's why ack's behavior of not searching things it doesn't recognize
  758;18 4:is one of its greatest strengths: the speed you get from only
  759:searching the things that you want to be looking at.
  760:
  --
  813:issues list at Github: L<http://github.com/petdance/ack/issues>
  814:
  815;45 4:Patches are always welcome, but patches with tests get the most
  816:attention.
  817:
  
  simpler output:
  :../Resources/ackmate_ack
  758;18 4:is one of its greatest strengths: the speed you get from only
  815;45 4:Patches are always welcome, but patches with tests get the most
 */

// One day this function will take the place of print_file_matches
void print_file_matches_with_context(const char* path, const char* buf, const int buf_len, const match matches[], const int matches_len) {
    int line = 1;
    int column = 0;
    char *context_prev_lines[opts.before];
    int prev_line = 0;
    int last_prev_line = 0;
    int prev_line_offset = 0;
    int cur_match = 0;
    int in_a_match = 0;
    int lines_since_last_match = 1000000; // if I initialize this to INT_MAX it'll overflow
    int last_printed_match = 0;

    if (first_file_match == 0 && opts.ackmate == 0) {
        printf("\n");
    }
    first_file_match = 0;

    print_path(path);

    for (int i = 0; i < opts.before; i++) {
        context_prev_lines[i] = NULL;
    }

    for (int i = 0; i < buf_len && (cur_match < matches_len || lines_since_last_match == 0); i++) {
        if (i == matches[cur_match].start) {
            in_a_match = 1;

            if (cur_match > 0 && lines_since_last_match > (opts.before + opts.after) && opts.context) {
                printf("--\n");
            }

            if (lines_since_last_match > 0) {
                if (lines_since_last_match > opts.after) {
                    // We found the start of a match. print the previous line(s)
                    for (int j = 0; j < opts.before; j++) {
                        prev_line = (last_prev_line + j) % opts.before;
                        if (context_prev_lines[prev_line] != NULL) {
                            if (opts.ackmate) {
                                printf("%i:%s\n", line - (opts.before - j), context_prev_lines[prev_line]);
                            }
                            else {
                                printf("%i-%s\n", line - (opts.before - j), context_prev_lines[prev_line]);
                            }
                        }
                    }
                }

                if (opts.ackmate == 0) {
                    printf("%i:", line);
                    // print up to current char
                    for (int j = prev_line_offset; j < i; j++) {
                        putchar(buf[j]);
                    }
                }
            }

            lines_since_last_match = 0;
            if (opts.color) {
                printf("%s", colors_match);
            }
        }

        if (i == matches[cur_match].end) {
            // We found the end of a match.
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

        if (buf[i] == '\n') {
            if (context_prev_lines[last_prev_line] != NULL) {
                free(context_prev_lines[last_prev_line]);
            }
            //column will always be at least 1. we don't want to strcpy the \n
            context_prev_lines[last_prev_line] = strndup(&buf[prev_line_offset], column - 1);
            last_prev_line = (last_prev_line + 1) % opts.before;
        }

        if (buf[i] == '\n' || i == buf_len - 1) {
            if (opts.ackmate && lines_since_last_match == 0) {
                // print headers for ackmate to parse
                printf("%i;", line);
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
                // print up to current char
                for (int j = prev_line_offset; j < i; j++) {
                    putchar(buf[j]);
                }
                putchar('\n');
            }

            prev_line_offset = i + 1; // skip the newline
            line++;
            column = 0;
            lines_since_last_match++;
        }
    }
}
