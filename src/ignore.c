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
#include "wildmatch.h"

#ifdef _WIN32
#include <shlwapi.h>
#define fnmatch(x, y, z) (!PathMatchSpec(y, x))
#else
#include <fnmatch.h>
const int fnmatch_flags = FNM_PATHNAME;
#endif

/* TODO: build a huge-ass list of files we want to ignore by default (build cache stuff, pyc files, etc) */

const char *evil_hardcoded_ignore_files[] = {
    ".",
    "..",
    NULL
};

/* Warning: changing the first string will break skip_vcs_ignores. */
const char *ignore_pattern_files[] = {
    ".agignore",
    ".hgignore",
    ".svn",
    NULL
};

/* gitignore has some special rules about how to interpret their contents and
 * as such cannot be parsed as the other files */
const char *gitignore_pattern_files[] = {
    /* The order of the files should match the precedence order. Highest -> lowest. */
    ".gitignore",
    ".git/info/exclude",
    NULL
};

int is_empty(ignores *ig) {
    return (ig->extensions_len
            + ig->names_len
            + ig->slash_names_len
            + ig->regexes_len
            + ig->gitignores_len
            + ig->slash_regexes_len == 0);
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
    ig->slash_regexes = NULL;
    ig->slash_regexes_len = 0;
    ig->dirname = dirname;
    ig->dirname_len = dirname_len;
    ig->gitignores = NULL;
    ig->gitignores_len = 0;

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
    size_t i;
    free_strings(ig->extensions, ig->extensions_len);
    free_strings(ig->names, ig->names_len);
    free_strings(ig->slash_names, ig->slash_names_len);
    free_strings(ig->regexes, ig->regexes_len);
    free_strings(ig->slash_regexes, ig->slash_regexes_len);
    char *last_freed = NULL;
    for(i = 0; i < ig->gitignores_len; ++i){
        free(ig->gitignores[i]->pattern);
        if(ig->gitignores[i]->prepend[0] != '\0' && last_freed != ig->gitignores[i]->prepend){
            last_freed = ig->gitignores[i]->prepend;
            free(ig->gitignores[i]->prepend);
        }
    }
    if (ig->abs_path) {
        free(ig->abs_path);
    }
    free(ig);
}

static enum pattern_type get_pattern_type(char const * pattern){
    if (is_fnmatch(pattern)) {
        if (pattern[0] == '*' && pattern[1] == '.' && !(is_fnmatch(pattern + 2))) {
            return PT_extension;
        } else if (pattern[0] == '/') {
            return PT_slash_regex;
        } else {
            return PT_regex;
        }
    } else {
        if (pattern[0] == '/') {
            return PT_slash_name;
        } else {
            return PT_name;
        }
    }
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
    switch(get_pattern_type(pattern)){
        case PT_extension:
            patterns_p = &(ig->extensions);
            patterns_len = &(ig->extensions_len);
            pattern += 2;
            break;
        case PT_slash_regex:
            patterns_p = &(ig->slash_regexes);
            patterns_len = &(ig->slash_regexes_len);
            pattern++;
            pattern_len--;
            break;
        case PT_regex:
            patterns_p = &(ig->regexes);
            patterns_len = &(ig->regexes_len);
            break;
        case PT_slash_name:
            patterns_p = &(ig->slash_names);
            patterns_len = &(ig->slash_names_len);
            pattern++;
            pattern_len--;
            break;
        case PT_name:
            patterns_p = &(ig->names);
            patterns_len = &(ig->names_len);
            break;
        default:
            patterns_p = NULL; /* silence compiler warnings */
            patterns_len = NULL; /* silence compiler warnings */
            die("Unknown pattern type");
            break;
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

void load_git_ignore_patterns(ignores *ig, const char *path, char *path_to_pwd) {
    FILE *fp = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        log_debug("Skipping gitignore file %s: not readable", path);
        return;
    }

    char *line = NULL;
    char *cline = NULL; /* corrected line */
    ssize_t line_len = 0;
    size_t line_cap = 0;
    size_t line_count = 0;

    log_debug("Loading ignore file %s.", path);

    size_t path_len = strlen(path_to_pwd);

    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; /* kill the \n */
        }
        cline = line;

        ++ig->gitignores_len;
        ig->gitignores = ag_realloc(ig->gitignores,
                ig->gitignores_len * sizeof(struct gitignore));
        if (ig->gitignores == NULL) {
            die("Failed to calloc gitignores list");
        }
        struct gitignore *gi = ag_calloc(1, sizeof(struct gitignore));
        if (gi == NULL) {
            die("Failed calloc gitignore line");
        }
        int begins_backslash = (cline[0] == '\\');
        if(begins_backslash){
            ++cline;
            --line_len;
        }

        if(!begins_backslash && cline[0] == '!'){
            gi->action = PA_include;
            ++cline;
            --line_len;
        }else{
            gi->action = PA_exclude;
        }
        gi->pattern = ag_strndup(cline, line_len);
        gi->len = line_len;
        gi->prepend = path_to_pwd;
        gi->prepend_len = path_len;
        gi->type = get_pattern_type(cline);


        ig->gitignores[ig->gitignores_len-1] = gi;
        ++line_count;
        log_debug("added ignore pattern %s to %s", line,
            ig == root_ignores ? "root ignores" : ig->abs_path);
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
    size_t patterns_len = strlen(patterns);
    while (*patterns != '\0' && patterns < (entry + bytes_read)) {
        for (line_len = 0; line_len < patterns_len; line_len++) {
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
        patterns_len -= line_len + 1;
    }
    free(entry);
cleanup:
    free(dir_prop_base);
    free(key);
    fclose(fp);
}

static int ackmate_dir_match(const char *dir_name) {
    if (opts.ackmate_dir_filter == NULL) {
        return 0;
    }
    /* we just care about the match, not where the matches are */
    return pcre_exec(opts.ackmate_dir_filter, NULL, dir_name, strlen(dir_name), 0, 0, NULL, 0);
}

enum pis_action {
    PIS_include,
    PIS_exclude,
    PIS_reinclude ///< This is what happens with git patterns starting with !
};

/** This is the hottest code in Ag. 10-15% of all execution time is spent here.
 * @param ig pointer to ignore information for this folder.
 * @param path the path from command invocation to @ref filename.
 *             Ensure paths startig with ./ are translated into /
 * @param filename the current file to test ignore patterns agains.
 * @param skip_git true will skip the git pattern matching part
 *
 * @returns action determined by the ignore files
 */
static enum pis_action path_ignore_search(const ignores *ig, const char *path, const char *filename, int skip_git) {
    char *temp;
    size_t i;
    int match_pos;

    match_pos = binary_search(filename, ig->names, 0, ig->names_len);
    if (match_pos >= 0) {
        log_debug("file %s ignored because name matches static pattern %s", filename, ig->names[match_pos]);
        return PIS_exclude;
    }

    ag_asprintf(&temp, "%s/%s", path[0] == '.' ? path + 1 : path, filename);

    if (strncmp(temp, ig->abs_path, ig->abs_path_len) == 0) {
        char *slash_filename = temp + ig->abs_path_len;
        if (slash_filename[0] == '/') {
            slash_filename++;
        }
        match_pos = binary_search(slash_filename, ig->names, 0, ig->names_len);
        if (match_pos >= 0) {
            log_debug("file %s ignored because name matches static pattern %s", temp, ig->names[match_pos]);
            free(temp);
            return PIS_exclude;
        }

        match_pos = binary_search(slash_filename, ig->slash_names, 0, ig->slash_names_len);
        if (match_pos >= 0) {
            log_debug("file %s ignored because name matches slash static pattern %s", slash_filename, ig->slash_names[match_pos]);
            free(temp);
            return PIS_exclude;
        }

        for (i = 0; i < ig->names_len; i++) {
            char *pos = strstr(slash_filename, ig->names[i]);
            if (pos == slash_filename || (pos && *(pos - 1) == '/')) {
                pos += strlen(ig->names[i]);
                if (*pos == '\0' || *pos == '/') {
                    log_debug("file %s ignored because path somewhere matches name %s", slash_filename, ig->names[i]);
                    free(temp);
                    return PIS_exclude;
                }
            }
            log_debug("pattern %s doesn't match path %s", ig->names[i], slash_filename);
        }

        for (i = 0; i < ig->slash_regexes_len; i++) {
            if (fnmatch(ig->slash_regexes[i], slash_filename, fnmatch_flags) == 0) {
                log_debug("file %s ignored because name matches slash regex pattern %s", slash_filename, ig->slash_regexes[i]);
                free(temp);
                return PIS_exclude;
            }
            log_debug("pattern %s doesn't match slash file %s", ig->slash_regexes[i], slash_filename);
        }
    }

    for (i = 0; i < ig->regexes_len; i++) {
        if (fnmatch(ig->regexes[i], filename, fnmatch_flags) == 0) {
            log_debug("file %s ignored because name matches regex pattern %s", filename, ig->regexes[i]);
            free(temp);
            return PIS_exclude;
        }
        log_debug("pattern %s doesn't match file %s", ig->regexes[i], filename);
    }

    /* File might be INcluded by git, if that is the case no parent ignore
     * can re-ignore it, so we can skip the match */
    if(!skip_git){
        int match = 0;
        char const * pattern_type = NULL;
        char * fullpath = NULL;
        ag_asprintf(&fullpath, "%s/%s", path, filename);
        char *fullpath_no_dot = fullpath;
        char *fullpath_no_dotslash = fullpath;
        if(fullpath[0] == '.') {
            fullpath_no_dot = fullpath + 1;
            if(fullpath[1] == '/'){
                fullpath_no_dotslash = fullpath + 2;
            }
        }


        struct gitignore *gi = NULL;
        const char * last_prepend = NULL;
        for(i = 0; i < ig->gitignores_len; ++i){
            /* Iterate the gitignores list backwards because the last match tells you
             * whether the file is to be ignored */
            gi = ig->gitignores[ig->gitignores_len - i - 1];
            match = 0;
            if(last_prepend != gi->prepend && gi->prepend[0] != '\0'){
                last_prepend = gi->prepend;
                char * tmp = fullpath;
                ag_asprintf(&fullpath, "./%s/%s", gi->prepend, fullpath_no_dotslash);
                free(tmp);
                fullpath_no_dot = fullpath+1;
                fullpath_no_dotslash = fullpath+2;
            }
            switch(gi->type){
                case PT_name:
                    {
                        log_debug("name: %s against pattern %s %d %d", fullpath, gi->pattern, gi->action, gi->len);
                        char const * pos = strstr(fullpath, gi->pattern);
                        if(pos){
                            pos += gi->len - 1;
                            if(*pos == '\0' || *pos == '/'){
                                match = 1;
                                pattern_type = "name";
                            }
                        }
                    }
                    break;
                case PT_slash_name:
                    log_debug("slash name: %s against pattern %s", filename, gi->pattern);
                    if(strcmp(gi->pattern, filename) == 0){
                        match = 1;
                    } else if(strcmp(gi->pattern, fullpath_no_dot) == 0){
                        match = 1;
                    }
                    if(match){
                        pattern_type = "slash_name";
                    }
                    break;
                case PT_extension:
                case PT_regex:
                    log_debug("regex: %s against pattern %s", fullpath_no_dotslash, gi->pattern);
                    if(wildmatch(gi->pattern, fullpath_no_dotslash, 0, NULL) == WM_MATCH){
                        match = 1;
                        pattern_type = "regex";
                    }
                    break;
                case PT_slash_regex:
                    log_debug("slash regex: %s against pattern %s", fullpath_no_dot, gi->pattern);
                    if(wildmatch(gi->pattern, fullpath_no_dot, WM_PATHNAME, NULL) == WM_MATCH){
                        match = 1;
                        pattern_type = "slash_regex";
                    }
                    break;
                default:
                    log_err("Found gitignore pattern '%s' that has an unknown type '%d'!", gi->pattern, gi->type);
                    continue;
            }
            if(match){
                break;
            }
        }
        /* gi is always non-null if we had a match */
        if(match) {
            if(gi->action == PA_exclude){
                // printf("ignored\n");
                log_debug("file %s ignored because name matches pattern %s from git."
                        " Type: %s", fullpath, gi->pattern, pattern_type);
                free(temp);
                return PIS_exclude;
            }else if(gi->action == PA_include){
                log_debug("file %s un-ignored because name matches patten %s from git."
                        " Type: %s", fullpath, gi->pattern, pattern_type);
                return PIS_reinclude;
            }else{
                log_err("Unknown pattern '%s' type %d found in git ignore pattern for folder %s",
                        gi->pattern, gi->type, path);
            }
        }
    }else{
        log_debug("Skipped git ignore match!");
    }

    int rv = ackmate_dir_match(temp);
    free(temp);
    return rv == 0 ? (skip_git ? PIS_reinclude : PIS_include) : PIS_exclude;
}

/* This function is REALLY HOT. It gets called for every file */
int filename_filter(const char *path, const struct dirent *dir, void *baton) {
    const char *filename = dir->d_name;
/* TODO: don't call strlen on filename every time we call filename_filter() */
#ifdef HAVE_DIRENT_DNAMLEN
    size_t filename_len = dir->d_namlen;
#else
    size_t filename_len = strlen(filename);
#endif
    size_t i;
    scandir_baton_t *scandir_baton = (scandir_baton_t *)baton;
    const ignores *ig = scandir_baton->ig;
    const char *base_path = scandir_baton->base_path;
    const size_t base_path_len = scandir_baton->base_path_len;
    const char *path_start = path;
    char *temp;

    if (!opts.search_hidden_files && filename[0] == '.') {
        return 0;
    }

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
        log_debug("%s ignored because it's a named pipe", path);
        return 0;
    }

    if (opts.search_all_files && !opts.path_to_agignore) {
        return 1;
    }

    /* base_path always ends with "/\0" while path doesn't, so this is safe */
    for (i = 0; base_path[i] == path[i] && i < base_path_len; i++);
    path_start = path + i;
    log_debug("fname_filter: path_start %s filename %s", path_start, filename);

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

    enum pis_action last_ignore = PIS_include;
    while (ig != NULL) {
        if (strncmp(filename, "./", 2) == 0) {
            filename++;
            filename_len--;
        }

        if (extension) {
            int match_pos = binary_search(extension, ig->extensions, 0, ig->extensions_len);
            if (match_pos >= 0) {
                log_debug("file %s ignored because name matches extension %s", filename, ig->extensions[match_pos]);
                return 0;
            }
        }

        last_ignore = path_ignore_search(ig, path_start, filename, last_ignore == PIS_reinclude);
        if (last_ignore == PIS_exclude) {
            return 0;
        }

        if (is_directory(path, dir) && filename[filename_len - 1] != '/') {
            ag_asprintf(&temp, "%s/", filename);
            last_ignore = path_ignore_search(ig, path_start, temp, last_ignore == PIS_reinclude);
            free(temp);
            if (last_ignore == PIS_exclude) {
                return 0;
            }
        }
        ig = ig->parent;
    }

    log_debug("%s not ignored", filename);
    return 1;
}
