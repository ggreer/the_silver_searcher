#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS  12
#define SINGLE_EXT_LEN  20

#define MAX_FILENAMES   12
#define SINGLE_NAME_LEN 30

typedef struct {
    const char *name;
    const char *extensions[MAX_EXTENSIONS];
    const char *full_filenames[MAX_FILENAMES];
} lang_spec_t;

extern lang_spec_t langs[];

/**
 Return the language count.
 */
size_t get_lang_count(void);

/**
Convert a NULL-terminated array of language extensions
into a regular expression of the form 
'((^|[/\\])(full_name1|full_name2)|\.(extension1|extension2...))$'

Caller is responsible for freeing the returned string.
*/
char *make_lang_regex(char *ext_array, size_t num_exts,
                      char *full_filenames, size_t num_filenames);


/**
Combine multiple file type extensions into one array.

The combined result is returned through *exts*;
*exts* is one-dimension array, which can contain up to 100 extensions;
The number of extensions that *exts* actually contain is returned.
*/
size_t combine_file_extensions(size_t *extension_index, size_t len, char **exts);

/**
Combine multiple file names into one array.

The combined result is returned through *names*;
*names* is one-dimension array, which can contain up to 100 extensions;
The number of file names that *names* actually contain is returned.
*/
size_t combine_full_filenames(size_t *extension_index, size_t len, char **names);
#endif
