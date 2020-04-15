#include <stdarg.h>
#include <stdio.h>

#include "log.h"
#include "util.h"

pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;
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

    pthread_mutex_lock(&print_mtx);
    FILE *stream = out_fd;

    switch (level) {
        case LOG_LEVEL_DEBUG:
            fprintf(stream, "DEBUG: ");
            break;
        case LOG_LEVEL_MSG:
            fprintf(stream, "MSG: ");
            break;
        case LOG_LEVEL_WARN:
            fprintf(stream, "WARN: ");
            break;
        case LOG_LEVEL_ERR:
            stream = stderr;
            fprintf(stream, "ERR: ");
            break;
    }

    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
    pthread_mutex_unlock(&print_mtx);
}

void plog(const unsigned int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vplog(level, fmt, args);
    va_end(args);
}
