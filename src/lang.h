#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS 12

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
char *make_lang_regex(const char **extensions);

#endif
