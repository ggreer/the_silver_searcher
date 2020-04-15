#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "scandir.h"
#include "util.h"

#ifdef _WIN32
#include <shlwapi.h>
#define fnmatch(x, y, z) (!PathMatchSpec(y, x))
#else
#include <fnmatch.h>
const int fnmatch_flags = FNM_PATHNAME;
#endif

ignores *root_ignores;

/* TODO: build a huge-ass list of files we want to ignore by default (build cache stuff, pyc files, etc) */

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

/* Warning: changing the first two strings will break skip_vcs_ignores. */
const char *ignore_pattern_files[] = {
    ".ignore",
    ".gitignore",
    ".git/info/exclude",
    ".hgignore",
    NULL
};

int is_empty(ignores *ig) {
    return (ig->extensions_len + ig->names_len + ig->slash_names_len + ig->regexes_len + ig->slash_regexes_len == 0);
};

ignores *init_ignore(ignores *parent, const char *dirname, const size_t dirname_len) {
    ignores *ig = ag_malloc(sizeof(ignores));
    ig->extensions = NULL;
    ig->extensions_len = 0;
    ig->names = NULL;
    ig->names_len = 0;
    ig->slash_names = NULL;
    ig->slash_names_len = 0;
    ig->regexes = NULL;
    ig->regexes_len = 0;
    ig->invert_regexes = NULL;
    ig->invert_regexes_len = 0;
    ig->slash_regexes = NULL;
    ig->slash_regexes_len = 0;
    ig->dirname = dirname;
    ig->dirname_len = dirname_len;

    if (parent && is_empty(parent) && parent->parent) {
        ig->parent = parent->parent;
    } else {
        ig->parent = parent;
    }

    if (parent && parent->abs_path_len > 0) {
        ag_asprintf(&(ig->abs_path), "%s/%s", parent->abs_path, dirname);
        ig->abs_path_len = parent->abs_path_len + 1 + dirname_len;
    } else if (dirname_len == 1 && dirname[0] == '.') {
        ig->abs_path = ag_malloc(sizeof(char));
        ig->abs_path[0] = '\0';
        ig->abs_path_len = 0;
    } else {
        ag_asprintf(&(ig->abs_path), "%s", dirname);
        ig->abs_path_len = dirname_len;
    }
    return ig;
}

void cleanup_ignore(ignores *ig) {
    if (ig == NULL) {
        return;
    }
    free_strings(ig->extensions, ig->extensions_len);
    free_strings(ig->names, ig->names_len);
    free_strings(ig->slash_names, ig->slash_names_len);
    free_strings(ig->regexes, ig->regexes_len);
    free_strings(ig->invert_regexes, ig->invert_regexes_len);
    free_strings(ig->slash_regexes, ig->slash_regexes_len);
    if (ig->abs_path) {
        free(ig->abs_path);
    }
    free(ig);
}

void add_ignore_pattern(ignores *ig, const char *pattern) {
    int i;
    int pattern_len;

    /* Strip off the leading dot so that matches are more likely. */
    if (strncmp(pattern, "./", 2) == 0) {
        pattern++;
    }

    /* Kill trailing whitespace */
    for (pattern_len = strlen(pattern); pattern_len > 0; pattern_len--) {
        if (!isspace(pattern[pattern_len - 1])) {
            break;
        }
    }

    if (pattern_len == 0) {
        log_debug("Pattern is empty. Not adding any ignores.");
        return;
    }

    char ***patterns_p;
    size_t *patterns_len;
    if (is_fnmatch(pattern)) {
        if (pattern[0] == '*' && pattern[1] == '.' && strchr(pattern + 2, '.') && !is_fnmatch(pattern + 2)) {
            patterns_p = &(ig->extensions);
            patterns_len = &(ig->extensions_len);
            pattern += 2;
            pattern_len -= 2;
        } else if (pattern[0] == '/') {
            patterns_p = &(ig->slash_regexes);
            patterns_len = &(ig->slash_regexes_len);
            pattern++;
            pattern_len--;
        } else if (pattern[0] == '!') {
            patterns_p = &(ig->invert_regexes);
            patterns_len = &(ig->invert_regexes_len);
            pattern++;
            pattern_len--;
        } else {
            patterns_p = &(ig->regexes);
            patterns_len = &(ig->regexes_len);
        }
    } else {
        if (pattern[0] == '/') {
            patterns_p = &(ig->slash_names);
            patterns_len = &(ig->slash_names_len);
            pattern++;
            pattern_len--;
        } else {
            patterns_p = &(ig->names);
            patterns_len = &(ig->names_len);
        }
    }

    ++*patterns_len;

    char **patterns;

    /* a balanced binary tree is best for performance, but I'm lazy */
    *patterns_p = patterns = ag_realloc(*patterns_p, (*patterns_len) * sizeof(char *));
    /* TODO: de-dupe these patterns */
    for (i = *patterns_len - 1; i > 0; i--) {
        if (strcmp(pattern, patterns[i - 1]) > 0) {
            break;
        }
        patterns[i] = patterns[i - 1];
    }
    patterns[i] = ag_strndup(pattern, pattern_len);
    log_debug("added ignore pattern %s to %s", pattern,
              ig == root_ignores ? "root ignores" : ig->abs_path);
}

/* For loading git/hg ignore patterns */
void load_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s: not readable", path);
        return;
    }
    log_debug("Loading ignore file %s.", path);

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(ig, line);
    }

    free(line);
    fclose(fp);
}

static int ackmate_dir_match(const char *dir_name) {
    if (opts.ackmate_dir_filter == NULL) {
        return 0;
    }
    /* we just care about the match, not where the matches are */
    return pcre_exec(opts.ackmate_dir_filter, NULL, dir_name, strlen(dir_name), 0, 0, NULL, 0);
}

/* This is the hottest code in Ag. 10-15% of all execution time is spent here */
static int path_ignore_search(const ignores *ig, const char *path, const char *filename) {
    char *temp;
    int temp_start_pos;
    size_t i;
    int match_pos;

    match_pos = binary_search(filename, ig->names, 0, ig->names_len);
    if (match_pos >= 0) {
        log_debug("file %s ignored because name matches static pattern %s", filename, ig->names[match_pos]);
        return 1;
    }

    ag_asprintf(&temp, "%s/%s", path[0] == '.' ? path + 1 : path, filename);
    //ig->abs_path has its leading slash stripped, so we have to strip the leading slash
    //of temp as well
    temp_start_pos = (temp[0] == '/') ? 1 : 0;

    if (strncmp(temp + temp_start_pos, ig->abs_path, ig->abs_path_len) == 0) {
        char *slash_filename = temp + temp_start_pos + ig->abs_path_len;
        if (slash_filename[0] == '/') {
            slash_filename++;
        }
        match_pos = binary_search(slash_filename, ig->names, 0, ig->names_len);
        if (match_pos >= 0) {
            log_debug("file %s ignored because name matches static pattern %s", temp, ig->names[match_pos]);
            free(temp);
            return 1;
        }

        match_pos = binary_search(slash_filename, ig->slash_names, 0, ig->slash_names_len);
        if (match_pos >= 0) {
            log_debug("file %s ignored because name matches slash static pattern %s", slash_filename, ig->slash_names[match_pos]);
            free(temp);
            return 1;
        }

        for (i = 0; i < ig->names_len; i++) {
            char *pos = strstr(slash_filename, ig->names[i]);
            if (pos == slash_filename || (pos && *(pos - 1) == '/')) {
                pos += strlen(ig->names[i]);
                if (*pos == '\0' || *pos == '/') {
                    log_debug("file %s ignored because path somewhere matches name %s", slash_filename, ig->names[i]);
                    free(temp);
                    return 1;
                }
            }
            log_debug("pattern %s doesn't match path %s", ig->names[i], slash_filename);
        }

        for (i = 0; i < ig->slash_regexes_len; i++) {
            if (fnmatch(ig->slash_regexes[i], slash_filename, fnmatch_flags) == 0) {
                log_debug("file %s ignored because name matches slash regex pattern %s", slash_filename, ig->slash_regexes[i]);
                free(temp);
                return 1;
            }
            log_debug("pattern %s doesn't match slash file %s", ig->slash_regexes[i], slash_filename);
        }
    }

    for (i = 0; i < ig->invert_regexes_len; i++) {
        if (fnmatch(ig->invert_regexes[i], filename, fnmatch_flags) == 0) {
            log_debug("file %s not ignored because name matches regex pattern !%s", filename, ig->invert_regexes[i]);
            free(temp);
            return 0;
        }
        log_debug("pattern !%s doesn't match file %s", ig->invert_regexes[i], filename);
    }

    for (i = 0; i < ig->regexes_len; i++) {
        if (fnmatch(ig->regexes[i], filename, fnmatch_flags) == 0) {
            log_debug("file %s ignored because name matches regex pattern %s", filename, ig->regexes[i]);
            free(temp);
            return 1;
        }
        log_debug("pattern %s doesn't match file %s", ig->regexes[i], filename);
    }

    int rv = ackmate_dir_match(temp);
    free(temp);
    return rv;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(const char *path, const struct dirent *dir, void *baton) {
    const char *filename = dir->d_name;
    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }

    size_t i;
    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            return 0;
        }
    }

    if (!opts.follow_symlinks && is_symlink(path, dir)) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return 0;
    }

    if (is_named_pipe(path, dir)) {
        log_debug("%s ignored because it's a named pipe or socket", path);
        return 0;
    }

    if (opts.search_all_files && !opts.path_to_ignore) {
        return 1;
    }

    scandir_baton_t *scandir_baton = (scandir_baton_t *)baton;
    const char *path_start = scandir_baton->path_start;

    const char *extension = strchr(filename, '.');
    if (extension) {
        if (extension[1]) {
            // The dot is not the last character, extension starts at the next one
            ++extension;
        } else {
            // No extension
            extension = NULL;
        }
    }

#ifdef HAVE_DIRENT_DNAMLEN
    size_t filename_len = dir->d_namlen;
#else
    size_t filename_len = 0;
#endif

    if (strncmp(filename, "./", 2) == 0) {
#ifndef HAVE_DIRENT_DNAMLEN
        filename_len = strlen(filename);
#endif
        filename++;
        filename_len--;
    }

    const ignores *ig = scandir_baton->ig;

    while (ig != NULL) {
        if (extension) {
            int match_pos = binary_search(extension, ig->extensions, 0, ig->extensions_len);
            if (match_pos >= 0) {
                log_debug("file %s ignored because name matches extension %s", filename, ig->extensions[match_pos]);
                return 0;
            }
        }

        if (path_ignore_search(ig, path_start, filename)) {
            return 0;
        }

        if (is_directory(path, dir)) {
#ifndef HAVE_DIRENT_DNAMLEN
            if (!filename_len) {
                filename_len = strlen(filename);
            }
#endif
            if (filename[filename_len - 1] != '/') {
                char *temp;
                ag_asprintf(&temp, "%s/", filename);
                int rv = path_ignore_search(ig, path_start, temp);
                free(temp);
                if (rv) {
                    return 0;
                }
            }
        }
        ig = ig->parent;
    }

    log_debug("%s not ignored", filename);
    return 1;
}
