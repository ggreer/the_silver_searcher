#ifndef LANG_H
#define LANG_H

#define MAX_EXTENSIONS 100

typedef struct {
    const char* language;
    const char* extensions[MAX_EXTENSIONS];
} language_specification;


extern language_specification languages[];

/**
Convert a NULL-terminated array of language extensions
into a regular expression of the form \.(extension1|extension2...)$

Caller is responsible for freeing the returned string.
*/
char* make_language_regex(const char** extensions);

int language_count();

#endif
