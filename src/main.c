#include <pcre.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"

#include "log.h"
#include "options.h"
#include "search.h"


int main(int argc, char **argv) {
    char **paths = NULL;
    int i;
    int pcre_opts = PCRE_MULTILINE;
    int study_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    double time_diff = 0.0;
    pthread_t *workers = NULL;
    int workers_len = 0;

    set_log_level(LOG_LEVEL_WARN);

    work_queue = NULL;
    work_queue_tail = NULL;
    workers_len = (int)sysconf(_SC_NPROCESSORS_ONLN);
    done_adding_files = FALSE;
    workers = calloc(workers_len, sizeof(pthread_t));
    if (pthread_cond_init(&files_ready, NULL)) {
        log_err("pthread_cond_init failed!");
        exit(2);
    }
    if (pthread_mutex_init(&work_queue_mtx, NULL)) {
        log_err("pthread_mutex_init failed!");
        exit(2);
    }
    memset(&stats, 0, sizeof(stats));
    root_ignores = init_ignore(NULL);
#ifdef USE_PCRE_JIT
    int has_jit = 0;
    pcre_config(PCRE_CONFIG_JIT, &has_jit);
    if (has_jit) {
        study_opts |= PCRE_STUDY_JIT_COMPILE;
    }
#endif

    gettimeofday(&(stats.time_start), NULL);

    parse_options(argc, argv, &paths);
    log_debug("PCRE Version: %s", pcre_version());
    log_debug("Using %i workers", workers_len);

    if (opts.casing == CASE_INSENSITIVE) {
        pcre_opts = pcre_opts | PCRE_CASELESS;
    }

    if (opts.literal) {
        generate_skip_lookup(opts.query, opts.query_len, skip_lookup, opts.casing == CASE_SENSITIVE);
    }
    else {
        if (opts.word_regexp) {
            opts.query_len = opts.query_len + 5; /* "\b" + "\b" + '\0' */
            char *word_regexp_query = malloc(opts.query_len);
            char *word_sep = "\\b";
            strlcpy(word_regexp_query, word_sep, opts.query_len);
            strlcat(word_regexp_query, opts.query, opts.query_len);
            strlcat(word_regexp_query, word_sep, opts.query_len);
            free(opts.query);
            opts.query = word_regexp_query;
            log_debug("Word regexp query: %s", opts.query);
        }
        opts.re = pcre_compile(opts.query, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
        if (opts.re == NULL) {
            log_err("pcre_compile failed at position %i. Error: %s", pcre_err_offset, pcre_err);
            exit(2);
        }
        opts.re_extra = pcre_study(opts.re, study_opts, &pcre_err);
        if (opts.re_extra == NULL) {
            log_debug("pcre_study returned nothing useful. Error: %s", pcre_err);
        }
    }

    if (opts.search_stream) {
        search_stream(stdin, "");
    }
    else {
        for (i = 0; i < workers_len; i++) {
            pthread_create(&(workers[i]), NULL, &search_file_worker, NULL);
        }
        for (i = 0; paths[i] != NULL; i++) {
            log_debug("searching path %s for %s", paths[i], opts.query);
            search_dir(root_ignores, paths[i], 0);
        }
        done_adding_files = TRUE;
        for (i = 0; i < workers_len; i++) {
            if (pthread_join(workers[i], NULL)) {
                log_err("pthread_join failed!");
                exit(2);
            }
        }
    }

    if (opts.stats) {
        gettimeofday(&(stats.time_end), NULL);
        time_diff = ((long)stats.time_end.tv_sec * 1000000 + stats.time_end.tv_usec) -
                    ((long)stats.time_start.tv_sec * 1000000 + stats.time_start.tv_usec);
        time_diff = time_diff / 1000000;

        printf("%ld matches\n%ld files searched\n%ld bytes searched\n%f seconds\n", stats.total_matches, stats.total_files, stats.total_bytes, time_diff);
    }

    pthread_cond_destroy(&files_ready);
    pthread_mutex_destroy(&work_queue_mtx);
    pcre_free(opts.re);
    if (opts.re_extra) {
        pcre_free(opts.re_extra); /* Using pcre_free_study here segfaults on some versions of PCRE */
    }
    cleanup_ignore(root_ignores);
    free(workers);
    free(paths);
    return 0;
}
