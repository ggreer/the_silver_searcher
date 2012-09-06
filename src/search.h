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

size_t skip_lookup[256];

struct work_queue_t {
    char *path;
    struct work_queue_t *next;
};
typedef struct work_queue_t work_queue_t;

work_queue_t *work_queue;
int done_adding_files;
pthread_cond_t files_ready;
pthread_mutex_t work_queue_mtx;

void search_buf(const char *buf, const int buf_len,
                const char *dir_full_path);
void search_stream(FILE *stream, const char *path);
void search_file(const char *file_full_path);

void *search_file_worker(void *void_args);

void search_dir(ignores *ig, const char* path, const int depth);

#endif
