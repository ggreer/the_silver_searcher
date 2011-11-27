#include <stdio.h>
#include <stdarg.h>

#include "log.h"

enum log_level log_threshold = LOG_LEVEL_DEBUG;

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

void vplog(const int level, const char *fmt, va_list args) {
    if (level < log_threshold) {
        return;
    }

    vprintf(fmt, args);
}

void plog(const int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(level, fmt, args);
    va_end(args);
}
