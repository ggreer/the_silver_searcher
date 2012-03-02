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

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "print.h"
#include "util.h"

ag_stats stats;

size_t skip_lookup[256];

int search_buf();
int search_stdin(const pcre *re, const pcre_extra *re_extra);
int search_dir(const pcre *re, const pcre_extra *re_extra, const char* path, const int depth);

#endif
