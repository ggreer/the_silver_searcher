#ifndef OPTIONS_H
#define OPTIONS_H

enum case_behavior {
    CASE_SENSITIVE,
    CASE_INSENSITIVE,
    CASE_SENSITIVE_RETRY_INSENSITIVE
};

typedef struct {
    int recurse_dirs;
    enum case_behavior casing;
} cli_options;

#endif
