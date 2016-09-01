#include <stdarg.h>
#include <stdio.h>

#include "log.h"
#include "util.h"

static enum log_level log_threshold = LOG_LEVEL_ERR;

void set_log_level(enum log_level threshold) {
    log_threshold = threshold;
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void log_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(LOG_LEVEL_MSG, fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void log_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(LOG_LEVEL_ERR, fmt, args);
    va_end(args);
}

void vplog(const unsigned int level, const char *fmt, va_list args) {
    if (level < log_threshold) {
        return;
    }

    FILE *stream = out_fd;

    char *level_fmt = malloc(strlen(fmt) + 9);
    level_fmt[0] = 0;
    switch (level) {
        case LOG_LEVEL_DEBUG:
            strcat(level_fmt, "DEBUG: ");
            break;
        case LOG_LEVEL_MSG:
            strcat(level_fmt, "MSG: ");
            break;
        case LOG_LEVEL_WARN:
            strcat(level_fmt, "WARN: ");
            break;
        case LOG_LEVEL_ERR:
            stream = stderr;
            strcat(level_fmt, "ERR: ");
            break;
    }
    strcat(level_fmt, fmt);
    strcat(level_fmt, "\n");

    vfprintf(stream, level_fmt, args);

    free(level_fmt);
}

void plog(const unsigned int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(level, fmt, args);
    va_end(args);
}
