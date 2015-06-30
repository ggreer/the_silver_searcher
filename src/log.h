#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

enum log_level {
    LOG_LEVEL_DEBUG = 10,
    LOG_LEVEL_MSG = 20,
    LOG_LEVEL_WARN = 30,
    LOG_LEVEL_ERR = 40,
    LOG_LEVEL_NONE = 100
};


void set_log_level(enum log_level threshold);

/*
 * if -D not set on command line, make a call to log_debug() turn into a
 * test that optimizes away the expense of actually calling the function.
 */
#define LOG_NULL__(x) (x == x)
#define log_debug(...) \
      opts.debug ? log_debug_actual(__VA_ARGS__) : LOG_NULL__(0)

void log_debug_actual(const char *fmt, ...);
void log_msg(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_err(const char *fmt, ...);

void vplog(const unsigned int level, const char *fmt, va_list args);
void plog(const unsigned int level, const char *fmt, ...);

#endif
