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
#ifdef _WIN32
# include <windows.h>
#else
# include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"
#include "uthash.h"

size_t skip_lookup[256];

struct work_queue_t {
    char *path;
    struct work_queue_t *next;
};
typedef struct work_queue_t work_queue_t;

work_queue_t *work_queue;
work_queue_t *work_queue_tail;
int done_adding_files;
pthread_cond_t files_ready;
pthread_mutex_t print_mtx;
pthread_mutex_t stats_mtx;
pthread_mutex_t work_queue_mtx;


/* For symlink loop detection */
#define SYMLOOP_ERROR   (-1)
#define SYMLOOP_OK      (0)
#define SYMLOOP_LOOP    (1)

typedef struct {
    dev_t dev;
    ino_t ino;
} dirkey_t;

typedef struct {
    dirkey_t key;
    UT_hash_handle hh;
} symdir_t;

symdir_t *symhash;

void search_buf(const char *buf, const off_t buf_len,
                const char *dir_full_path);
void search_stream(FILE *stream, const char *path);
void search_file(const char *file_full_path);

void *search_file_worker();

void search_dir(ignores *ig, const char *base_path, const char *path, const int depth);

#endif
