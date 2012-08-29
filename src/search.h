#ifndef SEARCH_H
#define SEARCH_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

#include "config.h"

ag_stats stats;

size_t skip_lookup[256];

void search_buf(const pcre *re, const pcre_extra *re_extra,
                const char *buf, const int buf_len,
                const char *dir_full_path);
void search_stdin(const pcre *re, const pcre_extra *re_extra);
void search_stream(const pcre *re, const pcre_extra *re_extra, FILE *stream, const char *path);
void search_file(const pcre *re, const pcre_extra *re_extra, const char *file_full_path);
void search_dir(ignores *ig, const pcre *re, const pcre_extra *re_extra, const char* path, const int depth);

#endif
