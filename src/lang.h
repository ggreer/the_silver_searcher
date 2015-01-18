#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS 12
#define SINGLE_EXT_LEN 20

typedef struct {
    const char *name;
    const char *extensions[MAX_EXTENSIONS];
} lang_spec_t;

extern lang_spec_t langs[];

/**
 Return the language count.
 */
unsigned int get_lang_count(void);

/**
Convert a NULL-terminated array of language extensions
into a regular expression of the form \.(extension1|extension2...)$

Caller is responsible for freeing the returned string.
*/
char *make_lang_regex(char *ext_array, unsigned int num_exts);


/**
Combine multiple file type extensions into one array.

The combined result is returned through *exts*;
*exts* is one-dimension array, which can contain up to 100 extensions;
The number of extensions that *exts* actually contain is returned.
*/
unsigned int
combine_file_extensions(unsigned int*   extension_index,
                        unsigned int    len,
                        char**          exts);
#endif
