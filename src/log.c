#include <stdio.h>
#include <stdarg.h>

#include "log.h"

enum log_level log_threshold = LOG_LEVEL_DEBUG;

void set_log_level(enum log_level threshold) {
    log_threshold = threshold;
}

void plog(const int level, const char *fmt, ...) {
    if (level < log_threshold) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
