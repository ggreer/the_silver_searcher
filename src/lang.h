#ifndef LANG_H
#define LANG_H

#define MAX_PATTERNS 12
#define SINGLE_PAT_LEN 20

typedef struct {
    const char *name;
    const char *patterns[MAX_PATTERNS];
} lang_spec_t;

extern lang_spec_t langs[];

/**
 Return the language count.
 */
size_t get_lang_count(void);

/**
Convert a NULL-terminated array of language patterns
into a regular expression of the form (\.?pattern1|\.?pattern2...)$
Patterns that begin with ^ are interpreted as full names, those
without are interpreted as file extensions.

Caller is responsible for freeing the returned string.
*/
char *make_lang_regex(char *pat_array, size_t num_pats);


/**
Combine multiple file type patterns into one array.

The combined result is returned through *pats*;
*pats* is one-dimension array, which can contain up to 100 patterns;
The number of patterns that *pats* actually contain is returned.
*/
size_t combine_file_patterns(size_t *pattern_index, size_t len, char **pats);
#endif
