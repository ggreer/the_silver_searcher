#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ignore.h"
#include "log.h"
#include "options.h"
#include "scandir.h"
#include "util.h"

/* TODO: build a huge-ass list of files we want to ignore by default (build cache stuff, pyc files, etc) */

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

/* Warning: changing the first string will break skip_vcs_ignores. */
const char *ignore_pattern_files[] = {
    ".agignore",
    ".gitignore",
    ".git/info/exclude",
    ".hgignore",
    ".svn",
    NULL
};

ignores *init_ignore(ignores *parent) {
    ignores *ig = ag_malloc(sizeof(ignores));
    ig->regexes = NULL;
    ig->flags = NULL;
    ig->regexes_len = 0;
    ig->parent = parent;
    return ig;
}

void cleanup_ignore(ignores *ig) {
    size_t i;

    if (ig) {
        if (ig->regexes) {
            for (i = 0; i < ig->regexes_len; i++) {
                free(ig->regexes[i]);
            }
            free(ig->regexes);
        }
        free(ig);
    }
}

typedef enum {
    normal, charclass, alternation
} state;

static char* resize_if_necessary (char* str, int pos, int* capacity) {
    if (pos >= *capacity) {
        *capacity *= 2;
        return realloc(str, *capacity);
    }
    return str;
}

static pcre* glob_to_regex(const char* glob, int glob_len, int* flags) {

    int capacity = glob_len + 8;
    char* regex = malloc(capacity);
    int pos = 0;

    state st = normal;
    int i = 0;
    if (glob[i] == '!') {
        *flags |= IGNORE_FLAG_INVERT;
        i++;
    } else if (glob[i] == '\\' && glob[i + 1] == '!') {
        /* handle escaped ! */
        i++;
    }
    if (glob[i] == '/') {
        /* anchored regexp */
        regex[pos++] = '^';
    } else {
        /* must match at start of filename or string */
        strcpy(regex, "(/|^)");
        pos = strlen(regex);
    }

    for (; i < glob_len; ++i) {
        char c = glob[i];
        switch (st) {
        case normal:
            switch(c) {
            case '.':
            case '|':
            case '+':
            case '^':
            case '$':
            case '(':
            case ')':
                /* escape regexp metacharacters */
                regex = resize_if_necessary(regex, pos + 2, &capacity);
                regex[pos++] = '\\';
                regex[pos++] = c;
                break;
            case '[':
                regex = resize_if_necessary(regex, pos + 2, &capacity);
                regex[pos++] = '[';
                if (glob[i+1] == '!') {
                    regex[pos++] = '^';
                    i++;
                }
                st = charclass;
                /* We are treating an initial caret as per regex,
                 since this is what bash does. */
                break;
            case '{':
                regex = resize_if_necessary(regex, pos + 1, &capacity);
                regex[pos++] = '(';
                st = alternation;
                break;
            case '?':
                regex = resize_if_necessary(regex, pos + 4, &capacity);
                strcpy(regex + pos, "[^/]");
                pos += 4;
                break;
            case '*':
                regex = resize_if_necessary(regex, pos + 5, &capacity);
                strcpy(regex + pos, "[^/]*");
                pos += 5;
                break;
            default:
                regex = resize_if_necessary(regex, pos + 1, &capacity);
                regex[pos++] = c;
                break;
            }
            break;
        case charclass:
            regex = resize_if_necessary(regex, pos + 1, &capacity);
            if (c == ']') {
                regex[pos++] = ']';
                st = normal;
            } else {
                regex[pos++] = c;
            }
            break;
        case alternation:
            regex = resize_if_necessary(regex, pos + 1, &capacity);
            switch (c) {
            case '.':
            case '|':
            case '+':
            case '^':
            case '$':
            case '(':
            case ')':
                /* escape regexp metacharacters */
                regex = resize_if_necessary(regex, pos + 2, &capacity);
                regex[pos++] = '\\';
                regex[pos++] = c;
                break;
            case '}':
                regex[pos++] = ')';
                st = normal;
                break;
            case ',':
                regex[pos++] = '|';
                break;
            default:
                regex[pos++] = c;
                break;
            }
            break;
        }
    }

    switch(st) {
    case charclass:
        regex = resize_if_necessary(regex, pos + 1, &capacity);
        regex[pos++] = ']';
        break;
    case alternation:
        regex = resize_if_necessary(regex, pos + 1, &capacity);
        regex[pos++] = ')';
        break;
    case normal:
        if (regex[pos - 1] == '/') {
            pos--;
            *flags |= IGNORE_FLAG_ISDIR;
        }
        break;
    }
    regex = resize_if_necessary(regex, pos + 2, &capacity);
    regex[pos++] = '$';
    regex[pos] = 0;

    int pcre_opts = 0;
    const char *pcre_err = NULL;
    int pcre_err_offset = 0;
    pcre *result = pcre_compile(regex, pcre_opts, &pcre_err, &pcre_err_offset, NULL);
    log_debug("regex for glob %s = %s flags %d", glob, regex, *flags);
    free(regex);
    return result;
}

void add_ignore_pattern(ignores *ig, const char* pattern) {
    int pattern_len;

    /* Treat ./ the same as /. */
    if (strncmp(pattern, "./", 2) == 0) {
        pattern += 1;
    }

    /* Kill trailing whitespace */
    for (pattern_len = strlen(pattern); pattern_len > 0; pattern_len--) {
        if (!isspace(pattern[pattern_len-1])) {
            break;
        }
    }

    if (pattern_len == 0) {
        log_debug("Pattern is empty. Not adding any ignores.");
        return;
    }

    /* TODO: de-dupe these patterns */
    ig->regexes_len++;
    ig->regexes = ag_realloc(ig->regexes, ig->regexes_len * sizeof(char*));
    ig->flags = ag_realloc(ig->flags, ig->regexes_len * sizeof(int));
    ig->regexes[ig->regexes_len - 1] = glob_to_regex(pattern, pattern_len, &(ig->flags[ig->regexes_len - 1]));
    log_debug("added regex ignore pattern %s", pattern);
}

/* For loading git ignore patterns;
   fixme: do hgignores (native regexps) too
 */
void load_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        log_debug("Skipping ignore file %s", path);
        return;
    }

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len-1] == '\n') {
            line[line_len-1] = '\0'; /* kill the \n */
        }
        add_ignore_pattern(ig, line);
    }

    free(line);
    fclose(fp);
}

void load_svn_ignore_patterns(ignores *ig, const char *path) {
    FILE *fp = NULL;
    char *dir_prop_base;
    ag_asprintf(&dir_prop_base, "%s/%s", path, SVN_DIR_PROP_BASE);

    fp = fopen(dir_prop_base, "r");
    if (fp == NULL) {
        log_debug("Skipping svn ignore file %s", dir_prop_base);
        free(dir_prop_base);
        return;
    }

    char *entry = NULL;
    size_t entry_len = 0;
    char *key = ag_malloc(32); /* Sane start for max key length. */
    size_t key_len = 0;
    size_t bytes_read = 0;
    char *entry_line;
    size_t line_len;
    int matches;

    while (fscanf(fp, "K %zu\n", &key_len) == 1) {
        key = ag_realloc(key, key_len + 1);
        bytes_read = fread(key, 1, key_len, fp);
        key[key_len] = '\0';
        matches = fscanf(fp, "\nV %zu\n", &entry_len);
        if (matches != 1) {
            log_debug("Unable to parse svnignore file %s: fscanf() got %i matches, expected 1.", dir_prop_base, matches);
            goto cleanup;
        }

        if (strncmp(SVN_PROP_IGNORE, key, bytes_read) != 0) {
            log_debug("key is %s, not %s. skipping %u bytes", key, SVN_PROP_IGNORE, entry_len);
            /* Not the key we care about. fseek and repeat */
            fseek(fp, entry_len + 1, SEEK_CUR); /* +1 to account for newline. yes I know this is hacky */
            continue;
        }
        /* Aww yeah. Time to ignore stuff */
        entry = ag_malloc(entry_len + 1);
        bytes_read = fread(entry, 1, entry_len, fp);
        entry[bytes_read] = '\0';
        log_debug("entry: %s", entry);
        break;
    }
    if (entry == NULL) {
        goto cleanup;
    }
    char *patterns = entry;
    while (*patterns != '\0' && patterns < (entry + bytes_read)) {
        for (line_len = 0; line_len < strlen(patterns); line_len++) {
            if (patterns[line_len] == '\n') {
                break;
            }
        }
        if (line_len > 0) {
            entry_line = ag_strndup(patterns, line_len);
            add_ignore_pattern(ig, entry_line);
            free(entry_line);
        }
        patterns += line_len + 1;
    }
    free(entry);
    cleanup:;
    free(dir_prop_base);
    free(key);
    fclose(fp);
}

static int ackmate_dir_match(const char* dir_name) {
    int rc = 0;

    if (opts.ackmate_dir_filter != NULL) {
        /* we just care about the match, not where the matches are */
        rc = pcre_exec(opts.ackmate_dir_filter, NULL, dir_name, strlen(dir_name), 0, 0, NULL, 0);
        if (rc >= 0) {
            log_debug("file %s ignored because name matches ackmate dir filter pattern", dir_name);
            return 1;
        }
    }

    return 0;
}

static int ignore_search(const ignores *ig, const char *path, const char *filename, int isdir) {

    char* qualified;
    ag_asprintf(&qualified, "%s/%s", path, filename);
    int qualified_len = strlen(qualified);

    log_debug("filter: %s -- %s = %s\n", path, filename, qualified);
    int ignored = 0;
    size_t i;
    for (i = 0; i < ig->regexes_len; ++i) {
        if (isdir || !(ig->flags[i] & IGNORE_FLAG_ISDIR)) {
            if (pcre_exec(ig->regexes[i], NULL, qualified, qualified_len, 0, 0, NULL, 0) > -1) {
                ignored = !(ig->flags[i] & IGNORE_FLAG_INVERT);
                log_debug("Setting ignore on %s to %d\n", qualified, ignored);
            }
        }
    }
    if (ignored) {
        log_debug("file %s ignored because name matches regexp pattern", qualified);
        goto done;
    }

    /* TODO: check that this is correct given the refactor */
    if (ackmate_dir_match(qualified)) {
        log_debug("file %s ignored because name matches ackmate regex", filename);
        ignored = TRUE;
        goto done;
    }

    log_debug("file %s not ignored", filename);

 done:
    free(qualified);
    return ignored;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(const char *path, const struct dirent *dir, void *baton) {
    const char *filename = dir->d_name;
    size_t i;
    scandir_baton_t *scandir_baton = (scandir_baton_t*) baton;
    const ignores *ig = scandir_baton->ig;
    const char *base_path = scandir_baton->base_path;
    const char *path_start = path;

    if (!opts.follow_symlinks && is_symlink(path, dir)) {
        log_debug("File %s ignored becaused it's a symlink", dir->d_name);
        return 0;
    }

    for (i = 0; evil_hardcoded_ignore_files[i] != NULL; i++) {
        if (strcmp(filename, evil_hardcoded_ignore_files[i]) == 0) {
            return 0;
        }
    }

    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }
    if (opts.search_all_files && !opts.path_to_agignore) {
        return 1;
    }

    for (i = 0; base_path[i] == path[i] && i < strlen(base_path); i++) {
        /* base_path always ends with "/\0" while path doesn't, so this is safe */
        path_start = path + i + 2;
    }
    log_debug("path_start is %s", path_start);

    /* The subpath is the portion of the path at which scandir_baton->ig's
     ignores were found */
    const char* subpath_start = path_start + strlen(path_start);
    for (i = 0; i < scandir_baton->level; ++i) {
        while (*subpath_start-- != '/') {
            if (subpath_start <= path_start) {
                log_debug("Overflowed while trying to find subpath");
            }
        }
    }
    if (scandir_baton->level)
        subpath_start++;

    int is_dir = is_directory(filename, dir);

    if (ignore_search(ig, subpath_start, filename, is_dir)) {
        return 0;
    }

    if (ig->parent != NULL) {
        scandir_baton_t new_baton = *scandir_baton;
        new_baton.level ++;
        new_baton.ig = ig->parent;
        int result = filename_filter(path, dir, (void *)&new_baton);
        return result;
    }

    return 1;
}
