#ifndef LOG_H
#define LOG_H

enum log_level {
    LOG_LEVEL_DEBUG = 10,
    LOG_LEVEL_MSG   = 20,
    LOG_LEVEL_WARN  = 30,
    LOG_LEVEL_ERR   = 40
};

void plog(const int level, const char *fmt, ...);

#endif
