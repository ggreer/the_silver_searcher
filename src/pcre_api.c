/******************************************************************************
 * Copyright (c) 2016 Allen Wild <allenwild93@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "pcre_api.h"
#include "util.h"

/*
 * Return the pcre version string
 */
const char *ag_pcre_version(void) {
#ifndef HAVE_PCRE2
    return pcre_version();
#else
    return AG_STRINGIFY(PCRE2_MAJOR) "." AG_STRINGIFY(PCRE2_MINOR) " " AG_STRINGIFY(PCRE2_DATE);
#endif
}

/*
 * Free the regex pointed to by re and set the pointer to NULL
 */
void ag_pcre_free_re(ag_pcre_re_t **re) {
    if (re && *re) {
#ifndef HAVE_PCRE2
        pcre_free(*re);
#else
        pcre2_code_free(*re);
#endif
        *re = NULL;
    }
}

/*
 * Free the regex pointed to by re and set the pointer to NULL
 */
void ag_pcre_free_extra(ag_pcre_extra_t **extra) {
    if (extra && *extra) {
#ifndef HAVE_PCRE2
        pcre_free(*extra);
#else
        pcre2_match_context_free(*extra);
#endif
        *extra = NULL;
    }
}

/*
 * This function used to be compile_study
 */
void ag_pcre_compile(ag_pcre_re_t **re, ag_pcre_extra_t **re_extra, char *q, const int pcre_opts, int use_jit) {
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;

#ifdef HAVE_PCRE2
    int pcre2_err = 0;
    char pcre2_err_buf[120] = { 0 };
    *re = pcre2_compile((PCRE2_UCHAR8 *)q, PCRE2_ZERO_TERMINATED, pcre_opts,
                        &pcre2_err, (PCRE2_SIZE *)&pcre_err_offset, NULL);
    if (*re == NULL) {
        pcre2_get_error_message(pcre2_err, (PCRE2_UCHAR8 *)pcre2_err_buf, sizeof(pcre2_err_buf));
        pcre_err = pcre2_err_buf;
    }
#else
    *re = pcre_compile(q, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
#endif

    if (*re == NULL) {
        die("Bad regex! pcre_compile() failed at position %i: %s\nIf you meant to search for a literal string, run ag with -Q",
            pcre_err_offset,
            pcre_err);
    }

#ifdef HAVE_PCRE2
    if (use_jit) {
        if (pcre2_jit_compile(*re, PCRE2_JIT_COMPLETE)) {
            log_warn("PCRE2 JIT compilation failed!");
        }
    }
    *re_extra = NULL;
#else
    if (use_jit) {
        use_jit = PCRE_STUDY_JIT_COMPILE;
    }
    *re_extra = pcre_study(*re, use_jit, &pcre_err);
    if (*re_extra == NULL) {
        log_debug("pcre_study returned nothing useful. Error: %s", pcre_err);
    }
#endif
}

/*
 * Run either pcre_match or pcre_exec
 */
int ag_pcre_match(ag_pcre_re_t *re, ag_pcre_extra_t *extra, const char *buf, int length,
                  int offset, int options, int *ovector, int ovecsize) {
    int rc;
#ifdef HAVE_PCRE2
    pcre2_match_data *match_data = pcre2_match_data_create(ovecsize, NULL);
    uint32_t ovec_count;
    PCRE2_SIZE *ovec_pointer;
    int i;

    rc = pcre2_match(re, (const PCRE2_UCHAR8 *)buf, (PCRE2_SIZE)length, offset, options, match_data, extra);
    ovec_count = pcre2_get_ovector_count(match_data);
    ovec_pointer = pcre2_get_ovector_pointer(match_data);
    for (i = 0; i < ovecsize && (uint32_t)i < ovec_count; i++) {
        ovector[i] = ovec_pointer[i];
    }
    pcre2_match_data_free(match_data);
#else
    rc = pcre_exec(re, extra, buf, length, offset, options, ovector, ovecsize);
#endif
    return rc;
}
