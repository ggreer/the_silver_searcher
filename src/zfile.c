#ifdef __FreeBSD__
#include <sys/endian.h>
#endif
#include <sys/types.h>

#ifdef __CYGWIN__
typedef _off64_t off64_t;
#endif

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#ifdef HAVE_ERR_H
#include <err.h>
#endif
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#ifdef HAVE_LZMA_H
#include <lzma.h>
#endif

#include "decompress.h"

#if HAVE_FOPENCOOKIE

#define min(a, b) ({				\
	__typeof (a) _a = (a);			\
	__typeof (b) _b = (b);			\
	_a < _b ? _a : _b; })

static cookie_read_function_t zfile_read;
static cookie_seek_function_t zfile_seek;
static cookie_close_function_t zfile_close;

static const cookie_io_functions_t zfile_io = {
    .read = zfile_read,
    .write = NULL,
    .seek = zfile_seek,
    .close = zfile_close,
};

#define KB (1024)
struct zfile {
    FILE *in;              // Source FILE stream
    uint64_t logic_offset, // Logical offset in output (forward seeks)
        decode_offset,     // Where we've decoded to
        actual_len;
    uint32_t outbuf_start;

    ag_compression_type ctype;

    union {
        z_stream gz;
        lzma_stream lzma;
    } stream;

    uint8_t inbuf[32 * KB];
    uint8_t outbuf[256 * KB];
    bool eof;
};

#define CAVAIL_IN(c) ((c)->ctype == AG_GZIP ? (c)->stream.gz.avail_in : (c)->stream.lzma.avail_in)
#define CNEXT_OUT(c) ((c)->ctype == AG_GZIP ? (c)->stream.gz.next_out : (c)->stream.lzma.next_out)

static int
zfile_cookie_init(struct zfile *cookie) {
#ifdef HAVE_LZMA_H
    lzma_ret lzrc;
#endif
    int rc;

    assert(cookie->logic_offset == 0);
    assert(cookie->decode_offset == 0);

    cookie->actual_len = 0;

    switch (cookie->ctype) {
#ifdef HAVE_ZLIB_H
        case AG_GZIP:
            memset(&cookie->stream.gz, 0, sizeof cookie->stream.gz);
            rc = inflateInit2(&cookie->stream.gz, 32 + 15);
            if (rc != Z_OK) {
                log_err("Unable to initialize zlib: %s", zError(rc));
                return EIO;
            }
            cookie->stream.gz.next_in = NULL;
            cookie->stream.gz.avail_in = 0;
            cookie->stream.gz.next_out = cookie->outbuf;
            cookie->stream.gz.avail_out = sizeof cookie->outbuf;
            break;
#endif
#ifdef HAVE_LZMA_H
        case AG_XZ:
            cookie->stream.lzma = (lzma_stream)LZMA_STREAM_INIT;
            lzrc = lzma_auto_decoder(&cookie->stream.lzma, -1, 0);
            if (lzrc != LZMA_OK) {
                log_err("Unable to initialize lzma_auto_decoder: %d", lzrc);
                return EIO;
            }
            cookie->stream.lzma.next_in = NULL;
            cookie->stream.lzma.avail_in = 0;
            cookie->stream.lzma.next_out = cookie->outbuf;
            cookie->stream.lzma.avail_out = sizeof cookie->outbuf;
            break;
#endif
        default:
            log_err("Unsupported compression type: %d", cookie->ctype);
            return EINVAL;
    }


    cookie->outbuf_start = 0;
    cookie->eof = false;
    return 0;
}

static void
zfile_cookie_cleanup(struct zfile *cookie) {
    switch (cookie->ctype) {
#ifdef HAVE_ZLIB_H
        case AG_GZIP:
            inflateEnd(&cookie->stream.gz);
            break;
#endif
#ifdef HAVE_LZMA_H
        case AG_XZ:
            lzma_end(&cookie->stream.lzma);
            break;
#endif
        default:
            /* Compiler false positive - unreachable. */
            break;
    }
}

/*
 * Open compressed file 'path' as a (forward-)seekable (and rewindable),
 * read-only stream.
 */
FILE *
decompress_open(int fd, const char *mode, ag_compression_type ctype) {
    struct zfile *cookie;
    FILE *res, *in;
    int error;

    cookie = NULL;
    in = res = NULL;
    if (strstr(mode, "w") || strstr(mode, "a")) {
        errno = EINVAL;
        goto out;
    }

    in = fdopen(fd, mode);
    if (in == NULL)
        goto out;

    /*
	 * No validation of compression type is done -- file is assumed to
	 * match input.  In Ag, the compression type is already detected, so
	 * that's ok.
	 */
    cookie = malloc(sizeof *cookie);
    if (cookie == NULL) {
        errno = ENOMEM;
        goto out;
    }

    cookie->in = in;
    cookie->logic_offset = 0;
    cookie->decode_offset = 0;
    cookie->ctype = ctype;

    error = zfile_cookie_init(cookie);
    if (error != 0) {
        errno = error;
        goto out;
    }

    res = fopencookie(cookie, mode, zfile_io);

out:
    if (res == NULL) {
        if (in != NULL)
            fclose(in);
        if (cookie != NULL)
            free(cookie);
    }
    return res;
}

/*
 * Return number of bytes into buf, 0 on EOF, -1 on error.  Update stream
 * offset.
 */
static ssize_t
zfile_read(void *cookie_, char *buf, size_t size) {
    struct zfile *cookie = cookie_;
    size_t nb, ignorebytes;
    ssize_t total = 0;
    lzma_ret lzret;
    int ret;

    assert(size <= SSIZE_MAX);

    if (size == 0)
        return 0;

    if (cookie->eof)
        return 0;

    ret = Z_OK;
    lzret = LZMA_OK;

    ignorebytes = cookie->logic_offset - cookie->decode_offset;
    assert(ignorebytes == 0);

    do {
        size_t inflated;

        /* Drain output buffer first */
        while (CNEXT_OUT(cookie) >
               &cookie->outbuf[cookie->outbuf_start]) {
            size_t left = CNEXT_OUT(cookie) -
                          &cookie->outbuf[cookie->outbuf_start];
            size_t ignoreskip = min(ignorebytes, left);
            size_t toread;

            if (ignoreskip > 0) {
                ignorebytes -= ignoreskip;
                left -= ignoreskip;
                cookie->outbuf_start += ignoreskip;
                cookie->decode_offset += ignoreskip;
            }

            // Ran out of output before we seek()ed up.
            if (ignorebytes > 0)
                break;

            toread = min(left, size);
            memcpy(buf, &cookie->outbuf[cookie->outbuf_start],
                   toread);

            buf += toread;
            size -= toread;
            left -= toread;
            cookie->outbuf_start += toread;
            cookie->decode_offset += toread;
            cookie->logic_offset += toread;
            total += toread;

            if (size == 0)
                break;
        }

        if (size == 0)
            break;

        /*
		 * If we have not satisfied read, the output buffer must be
		 * empty.
		 */
        assert(cookie->stream.gz.next_out ==
               &cookie->outbuf[cookie->outbuf_start]);

        if ((cookie->ctype == AG_XZ && lzret == LZMA_STREAM_END) ||
            (cookie->ctype == AG_GZIP && ret == Z_STREAM_END)) {
            cookie->eof = true;
            break;
        }

        /* Read more input if empty */
        if (CAVAIL_IN(cookie) == 0) {
            nb = fread(cookie->inbuf, 1, sizeof cookie->inbuf,
                       cookie->in);
            if (ferror(cookie->in)) {
                warn("error read core");
                exit(1);
            }
            if (nb == 0 && feof(cookie->in)) {
                warn("truncated file");
                exit(1);
            }
            if (cookie->ctype == AG_XZ) {
                cookie->stream.lzma.avail_in = nb;
                cookie->stream.lzma.next_in = cookie->inbuf;
            } else {
                cookie->stream.gz.avail_in = nb;
                cookie->stream.gz.next_in = cookie->inbuf;
            }
        }

        /* Reset stream state to beginning of output buffer */
        if (cookie->ctype == AG_XZ) {
            cookie->stream.lzma.next_out = cookie->outbuf;
            cookie->stream.lzma.avail_out = sizeof cookie->outbuf;
        } else {
            cookie->stream.gz.next_out = cookie->outbuf;
            cookie->stream.gz.avail_out = sizeof cookie->outbuf;
        }
        cookie->outbuf_start = 0;

        if (cookie->ctype == AG_GZIP) {
            ret = inflate(&cookie->stream.gz, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END) {
                log_err("Found mem/data error while decompressing zlib stream: %s", zError(ret));
                return -1;
            }
        } else {
            lzret = lzma_code(&cookie->stream.lzma, LZMA_RUN);
            if (lzret != LZMA_OK && lzret != LZMA_STREAM_END) {
                log_err("Found mem/data error while decompressing xz/lzma stream: %d", lzret);
                return -1;
            }
        }
        inflated = CNEXT_OUT(cookie) - &cookie->outbuf[0];
        cookie->actual_len += inflated;
    } while (!ferror(cookie->in) && size > 0);

    assert(total <= SSIZE_MAX);
    return total;
}

static int
zfile_seek(void *cookie_, off64_t *offset_, int whence) {
    struct zfile *cookie = cookie_;
    off64_t new_offset = 0, offset = *offset_;

    if (whence == SEEK_SET) {
        new_offset = offset;
    } else if (whence == SEEK_CUR) {
        new_offset = (off64_t)cookie->logic_offset + offset;
    } else {
        /* SEEK_END not ok */
        return -1;
    }

    if (new_offset < 0)
        return -1;

    /* Backward seeks to anywhere but 0 are not ok */
    if (new_offset < (off64_t)cookie->logic_offset && new_offset != 0) {
        return -1;
    }

    if (new_offset == 0) {
        /* rewind(3) */
        cookie->decode_offset = 0;
        cookie->logic_offset = 0;
        zfile_cookie_cleanup(cookie);
        zfile_cookie_init(cookie);
    } else if ((uint64_t)new_offset > cookie->logic_offset) {
        /* Emulate forward seek by skipping ... */
        char *buf;
        const size_t bsz = 32 * 1024;

        buf = malloc(bsz);
        while ((uint64_t)new_offset > cookie->logic_offset) {
            size_t diff = min(bsz,
                              (uint64_t)new_offset - cookie->logic_offset);
            ssize_t err = zfile_read(cookie_, buf, diff);
            if (err < 0) {
                free(buf);
                return -1;
            }

            /* Seek past EOF gets positioned at EOF */
            if (err == 0) {
                assert(cookie->eof);
                new_offset = cookie->logic_offset;
                break;
            }
        }
        free(buf);
    }

    assert(cookie->logic_offset == (uint64_t)new_offset);

    *offset_ = new_offset;
    return 0;
}

static int
zfile_close(void *cookie_) {
    struct zfile *cookie = cookie_;

    zfile_cookie_cleanup(cookie);
    fclose(cookie->in);
    free(cookie);

    return 0;
}

#endif /* HAVE_FOPENCOOKIE */
