#include <stdio.h>
#include <stdarg.h>

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

    char *heading;
    char *full_fmt;
    FILE *stream = out_fd;

    switch (level) {
        case LOG_LEVEL_DEBUG:
            heading = "DEBUG: ";
            break;
        case LOG_LEVEL_MSG:
            heading = "MSG: ";
            break;
        case LOG_LEVEL_WARN:
            heading = "WARN: ";
            break;
        case LOG_LEVEL_ERR:
            stream = stderr;
            heading = "ERR: ";
            break;
    }
    full_fmt = (char *)ag_malloc(strlen(heading) + strlen(fmt) + 2);
    sprintf(full_fmt, "%s%s\n", heading, fmt);
    pthread_mutex_lock(&print_mtx);
    vfprintf(stream, full_fmt, args);
    pthread_mutex_unlock(&print_mtx);
    free(full_fmt);
}

void plog(const unsigned int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(level, fmt, args);
    va_end(args);
}
