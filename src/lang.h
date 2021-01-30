#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS 12
#define SINGLE_EXT_LEN 20

typedef struct {
    const char *name;
    const char *extensions[MAX_EXTENSIONS];
} lang_spec_t;

/**
 Parses additional lang_spec_t from a file
 */
void lang_parse_user_spec();

/**
 Returns languages directory
 */
lang_spec_t const *get_langs(void);

/**
 Return the language count.
 */
size_t get_lang_count(void);

/**
Convert a NULL-terminated array of language extensions
into a regular expression of the form \.(extension1|extension2...)$

Caller is responsible for freeing the returned string.
*/
char *make_lang_regex(char *ext_array, size_t num_exts);


/**
Combine multiple file type extensions into one array.

The combined result is returned through *exts*;
*exts* is one-dimension array, which can contain up to 100 extensions;
The number of extensions that *exts* actually contain is returned.
*/
size_t combine_file_extensions(size_t *extension_index, size_t len, char **exts);
#endif
