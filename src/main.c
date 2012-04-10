#include <pcre.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "options.h"
#include "search.h"

#include "config.h"

int main(int argc, char **argv) {
    set_log_level(LOG_LEVEL_WARN);

    char *query = NULL;
    char *path = NULL;
    int pcre_opts = PCRE_MULTILINE;
    int study_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *re = NULL;
    pcre_extra *re_extra = NULL;
    double time_diff = 0.0;

    memset(&stats, 0, sizeof(stats)); /* What's the point of an init function if it's going to be this one-liner? */

    gettimeofday(&(stats.time_start), NULL);

    parse_options(argc, argv, &query, &path);

    log_debug("PCRE Version: %s", pcre_version());

    if (opts.casing == CASE_INSENSITIVE) {
        pcre_opts = pcre_opts | PCRE_CASELESS;
    }

    if (!is_regex(opts.query)) {
        /* No special chars. Do a literal match */
        opts.literal = 1;
    }

    if (opts.literal) {
        generate_skip_lookup(opts.query, opts.query_len, skip_lookup, opts.casing == CASE_SENSITIVE);
    }
    else {
        re = pcre_compile(query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
        if (re == NULL) {
            log_err("pcre_compile failed at position %i. Error: %s", pcre_err_offset, pcre_err);
            exit(1);
        }

#ifdef USE_PCRE_JIT
        int has_jit = 0;
        pcre_config(PCRE_CONFIG_JIT, &has_jit);
        if (has_jit) {
            study_opts |= PCRE_STUDY_JIT_COMPILE;
        }
#endif

        re_extra = pcre_study(re, study_opts, &pcre_err);
        if (re_extra == NULL) {
            log_debug("pcre_study returned nothing useful. Error: %s", pcre_err);
        }
    }

    if (opts.search_stream) {
        search_stdin(re, re_extra);
    }
    else {
        search_dir(re, re_extra, path, 0);
    }

    if (opts.stats) {
        gettimeofday(&(stats.time_end), NULL);
        time_diff = ((long)stats.time_end.tv_sec * 1000000 + stats.time_end.tv_usec) -
                    ((long)stats.time_start.tv_sec * 1000000 + stats.time_start.tv_usec);
        time_diff = time_diff / 1000000;

        printf("%ld matches\n%ld files searched\n%ld bytes searched\n%f seconds\n", stats.total_matches, stats.total_files, stats.total_bytes, time_diff);
    }

    pcre_free(re);
    pcre_free(re_extra); /* Using pcre_free_study here segfaults on some versions of PCRE */
    free(query);
    free(path);
    cleanup_ignore_patterns();

    return(0);
}
