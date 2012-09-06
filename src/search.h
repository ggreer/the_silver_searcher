#ifndef SEARCH_H
#define SEARCH_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pcre.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

typedef struct {
    const pcre *re;
    const pcre_extra *re_extra;
    char *path;
} search_file_args;

size_t skip_lookup[256];

struct work_queue_t {
    char *path;
    struct work_queue_t *next;
};
typedef struct work_queue_t work_queue_t;

work_queue_t *work_queue;

void search_buf(const pcre *re, const pcre_extra *re_extra,
                const char *buf, const int buf_len,
                const char *dir_full_path);
void search_stdin(const pcre *re, const pcre_extra *re_extra);
void search_stream(const pcre *re, const pcre_extra *re_extra, FILE *stream, const char *path);
void search_file(const pcre *re, const pcre_extra *re_extra, const char *file_full_path);

int search_file_wrap(pthread_t *thread, const pcre *re, const pcre_extra *re_extra, char* path);
void *search_file_entry(void *void_args);
void search_dir(ignores *ig, const pcre *re, const pcre_extra *re_extra, const char* path, const int depth);

#endif
