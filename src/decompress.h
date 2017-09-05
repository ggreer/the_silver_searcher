#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include <stdio.h>

#include "config.h"
#include "log.h"
#include "options.h"

typedef enum {
    AG_NO_COMPRESSION,
    AG_GZIP,
    AG_COMPRESS,
    AG_ZIP,
    AG_XZ,
} ag_compression_type;

ag_compression_type is_zipped(const void *buf, const int buf_len);

void *decompress(const ag_compression_type zip_type, const void *buf, const int buf_len, const char *dir_full_path, int *new_buf_len);

#ifdef USE_FOPENCOOKIE
FILE *decompress_open(int fd, const char *mode, ag_compression_type ctype);
#endif

#endif
