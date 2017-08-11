#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#include "config.h"

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

extern pthread_mutex_t print_mtx;

enum log_level {
    LOG_LEVEL_DEBUG = 10,
    LOG_LEVEL_MSG = 20,
    LOG_LEVEL_WARN = 30,
    LOG_LEVEL_ERR = 40,
    LOG_LEVEL_NONE = 100
};

void set_log_level(enum log_level threshold);

void log_debug(const char *fmt, ...);
void log_msg(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_err(const char *fmt, ...);

void vplog(const unsigned int level, const char *fmt, va_list args);
void plog(const unsigned int level, const char *fmt, ...);

#endif
