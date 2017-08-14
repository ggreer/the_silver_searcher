#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS 12
#define SINGLE_EXT_LEN 20

#define MAX_NAMES 12
#define SINGLE_NAME_LEN 20

typedef struct {
    const char *name;
    const char *extensions[MAX_EXTENSIONS];
    const char *names[MAX_NAMES];
} lang_spec_t;

extern lang_spec_t langs[];

/**
 Return the language count.
 */
size_t get_lang_count(void);

/**
Convert a NULL-terminated array of language extensions
into a regular expression of the form \.(extension1|extension2...)$

Caller is responsible for freeing the returned string.
*/
char *make_lang_regex(char *ext_array, size_t num_exts, char *name_array, size_t num_names);


/**
Combine multiple file type extensions and names into one arrays.

The combined result is returned through *exts* and *names*;
both are one-dimension arrays, which can contain up to 100 extensions;
The number of extensions that *exts* and *names* contain are returned
through num_extensions and num_names.
*/
void combine_file_extensions(size_t *extension_index, size_t len,
                             size_t *num_extensions, char **exts,
                             size_t *num_names, char **names);
#endif
