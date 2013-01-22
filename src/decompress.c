#include <zlib.h>

#include "decompress.h"

/* Code in decompress_zlib from
 *
 * https://raw.github.com/madler/zlib/master/examples/zpipe.c
 *
 * zpipe.c: example of proper use of zlib's inflate() and deflate()
 *    Not copyrighted -- provided to the public domain
 *    Version 1.4  11 December 2005  Mark Adler 
 */


static void* decompress_zlib(const void* buf, const int buf_len,
                             const char* dir_full_path, int* new_buf_len) {
    int ret = 0;
    unsigned char* result = NULL;
    size_t result_size = 0;
    z_stream stream;

    log_debug("Decompressing zlib file %s", dir_full_path);

    /* allocate inflate state */
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;

    /* Add 32 to allow zlib and gzip format detection */
    if(inflateInit2(&stream, 32 + 15) != Z_OK) {
        log_err("Unable to initialize zlib: %s", stream.msg);
        goto error_out;
    }

    stream.avail_in = buf_len;
    stream.next_in = (void*)buf;

    result_size = buf_len;
    do {
        /* Double the buffer size and realloc */
        result_size *= 2;
        result = (unsigned char*)realloc(result, result_size * sizeof(unsigned char));

        stream.avail_out = *new_buf_len;
        stream.next_out = &result[stream.total_out];
        ret = inflate(&stream, Z_SYNC_FLUSH);
        log_debug("inflate ret = %d", ret);
        switch(ret) {
            case Z_STREAM_ERROR: {
                log_err("Found stream error while decompressing zlib stream: %s", stream.msg);
                inflateEnd(&stream);
                goto error_out;
            }
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR: {
                log_err("Found mem/data error while decompressing zlib stream: %s", stream.msg);
                inflateEnd(&stream);
                goto error_out;
            }
        }
    } while(stream.avail_out == 0);

    *new_buf_len = stream.total_out;
    inflateEnd(&stream);

    if(ret == Z_STREAM_END) {
        return result;
    }

error_out:
    *new_buf_len = 0;
    return NULL;
}

static void* decompress_lwz(const void* buf, const int buf_len,
                            const char* dir_full_path, int* new_buf_len) {
    (void)buf; (void)buf_len;
    log_err("LWZ (UNIX compress) files not yet supported: %s", dir_full_path);
    *new_buf_len = 0;
    return NULL;
}

static void* decompress_zip(const void* buf, const int buf_len,
                            const char* dir_full_path, int* new_buf_len) {
    (void)buf; (void)buf_len;
    log_err("Zip files not yet supported: %s", dir_full_path);
    *new_buf_len = 0;
    return NULL;
}


/* This function is very hot. It's called on every file when zip is enabled. */
void* decompress(const ag_compression_type zip_type, const void* buf, const int buf_len,
                 const char* dir_full_path, int* new_buf_len) {

    switch(zip_type) {
        case AG_GZIP:
             return decompress_zlib(buf, buf_len, dir_full_path, new_buf_len);
        case AG_COMPRESS:
             return decompress_lwz(buf, buf_len, dir_full_path, new_buf_len);
        case AG_ZIP:
             return decompress_zip(buf, buf_len, dir_full_path, new_buf_len);
        case AG_NO_COMPRESSION:
            log_err("File %s is not compressed", dir_full_path);
            break;
        default:
            log_err("Unsupported compression type: %d", zip_type);
    }

    *new_buf_len = 0;
    return NULL;
}


/* This function is very hot. It's called on every file. */
ag_compression_type is_zipped(const void* buf, const int buf_len) {
    /* Zip magic numbers
     * compressed file: { 0x1F, 0x9B }
     * http://en.wikipedia.org/wiki/Compress
     * 
     * gzip file:       { 0x1F, 0x8B }
     * http://www.gzip.org/zlib/rfc-gzip.html#file-format
     *
     * zip file:        { 0x50, 0x4B, 0x03, 0x04 }
     * http://www.pkware.com/documents/casestudies/APPNOTE.TXT (Section 4.3)
     */ 

    const unsigned char *buf_c = buf;

    if(buf_len == 0) {
        return AG_NO_COMPRESSION;
    }

    /* Check for gzip & compress */
    if(buf_len >= 2) {
        if(buf_c[0] == 0x1F) {
            if(buf_c[1] == 0x8B) {
                log_debug("Found gzip-based stream");
                return AG_GZIP;
            } else if(buf_c[1] == 0x9B) {
                log_debug("Found compress-based stream");
                return AG_COMPRESS;
            }
        }
    }

    /* Check for zip */
    if(buf_len >= 4) {
        if(buf_c[0] == 0x50 && buf_c[1] == 0x4B && buf_c[2] == 0x03 && buf_c[3] == 0x04)
        {
            log_debug("Found zip-based stream");
            return AG_ZIP;
        }
    }

    return AG_NO_COMPRESSION;
}

