/*
 * Implement option initialization via external config files. 
 *
 * Implementation is based on the work of David Schlosnagle in github PR
 * #257.  Additional features added based on the conversation in that same
 * PR.
 * 
 * Note that the naming prefix "ag_rc" was chosen specifically because the
 * shorter prefix "agrc" is very similar to the canonical command line var
 * "argc".  Selecting "ag_rc" minimizes error-prone code changes due to
 * simple typos and facilitates more meaningful third-party reviews.
 */

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "util.h"
#include "log.h"
#include "ag_rc.h"

static ag_rc_options *ao = NULL;

static const char *unix_paths[] = { 
        "/usr/local/etc/agrc", 
        "/etc/agrc", 
        NULL, 
    };

struct env_path_struct
{
    const char *var;
    const char *leaf;
};

static const struct env_path_struct env_paths[] = {
        { "XDG_CONFIG_HOME", "agrc"  },
        { "HOME",            ".agrc" },
        { NULL,              NULL    },
    };

static void add_file_option(char *);
static void load_file_options(FILE *, const char *);
static void parse_file_option(char *, const char *, int);
static char **update_argv(int *argc, char **argv);

/*
 * ============================  WARNING =========================
 *
 * Use of log_debug() in this module is a waste of time.  
 *
 * Reason:  command line options (including the -D switch) are not
 * parsed until after config file options are read and prepended
 * to argv.  Been there, done that.
 *
 * ============================  WARNING =========================
 */
 


/*
 * Read and parse external config file(s) and prepend options to argv.
 *
 * Returns
 * =======
 * - Amended argv, by value.
 *
 * - Amended argc by reference.
 *
 * - A flag (by reference) denoting whether or not the command "list"
 *   appeared in a config file.
 *
 * "list" causes Ag to dump the contents of argv[] on stdout.  Useful
 * for debugging errors in config files that cause Ag to output a usage
 * message and then silently die.
 *
 * Precedence order of where to look for a config file goes like so:
 *
 * 1) $AGRC_PATH
 * 2) $XDG_CONFIG_HOME/agrc
 * 3) $HOME/.agrc
 * 4) /usr/local/etc/agrc
 * 5) /etc/agrc
 *
 * First encountered config file that can be open'd "r" wins (is used).  All
 * others ignored.
 */
char **
ag_rc_read_options(int *argc, char **argv, int *list_argv)
{
    FILE *fp = NULL;

    ao = ag_calloc(1, sizeof(*ao));

    const char *ag_rc_path = getenv("AGRC_PATH");
    if (ag_rc_path) {
        if ((fp = fopen(ag_rc_path, "r")) == NULL) {
            log_err("Cannot read config file $AGRC_PATH(%s): %s", 
                     ag_rc_path, strerror(errno));
        } else {
            load_file_options(fp, ag_rc_path);
            goto cleanup;
        }
    }

    const struct env_path_struct *env;
    for (env = env_paths; env->var; env++) {
        const char *dirname = getenv(env->var);
        if (dirname) {
            char *full_path;

            ag_asprintf(&full_path, "%s/%s", dirname, env->leaf);
            if ((fp = fopen(full_path, "r")) != NULL) {
                load_file_options(fp, full_path);
                (void) free(full_path);
                goto cleanup;
            }
        }
    }

    const char **upath;
    for (upath = unix_paths; *upath; upath++) {
        if ((fp = fopen(*upath, "r")) != NULL) {
            load_file_options(fp, *upath);
            goto cleanup;
        }
    }

cleanup:
    if (fp) {
        (void) fclose(fp);
    }
    *list_argv = ao->list_argv;
    return (ao->options_len) ?  update_argv(argc, argv) : argv;
}

static void
load_file_options(FILE *fp, const char *path)
{
    int file_line_no = 0;

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
        parse_file_option(line, path, ++file_line_no);
    }

    free(line);
}

static void 
parse_file_option(char *option, const char *path, int line_no)
{
    size_t option_len;
    char   *value, *option_end;

    /* Kill leading whitespace */
    while (*option && isspace((int) *option)) {
        option++;
    }

    /* Exclude comments and blank lines */
    if (*option == '\0' || *option == '#') {
        return;
    }

    /* Kill trailing whitespace */
    for (option_len = strlen(option); option_len > 0; --option_len) {
        if (!isspace((int) option[option_len - 1])) {
            break;
        } else {
            option[option_len - 1] = '\0';
        }
    }

    /* Split options with values, allow \s+ or '\s+=\s+' as delimiter. */
    option_end = value = strchr(option, ' ');
    if (value) {
        value++;
        while (*value && isspace((int) *value)) {
            value++;
        }
        if (*value == '=') {
            value++;
            while (*value && isspace((int) *value)) {
                value++;
            }
            if (*value == '\0') {
                /* 
                 * Line read from config file is "<option>=". I.E., crap.
                 * Let the crap flow thru to the command line parser.
                 */

                value = NULL;
            }
        }
    } else {
        option_end = value = strchr(option, '=');
        if (value) {
            value++;
            while (*value && isspace((int) *value)) {
                value++;
            }
            if (*value == '\0') {
                value = NULL;  /* crap option */
            }
        }
    }

    if (value == NULL && strcmp(option, "list") == 0) {
        ao->list_argv = TRUE;  /* debug -- dump argv */
        return;
    }

    /*
     * -------------------- TODO maybe ---------------------
     * Add support (right here) for an "include" command
     * that enables chained config files.
     *
     * Maybe something like "include<space><filename>" .
     * The entire config file processing call chain is
     * re-entrant, so, should be doable as long as a test
     * is added for excessive include file depth (as opposed
     * to recursively popping the stack :-) ).
     * -------------------- TODO maybe ---------------------
     */

    /*
     * If option does not begin with '-', bark.  The cmdline parser will
     * consider anything that does not begin with '-' as a pattern and
     * after that it's off to the races, which is incredibly confusing.
     */
    if (*option != '-') {
        log_err("config file %s:%d: option \"%s\" not preceded with '-'",
                path,
                line_no,
                option);
        return;
    }

    if (value) {
        /* option and value specified */

        *option_end = '\0';   /* kill delimiter */
        add_file_option(ag_strdup(option));
        add_file_option(ag_strdup(value));
    }
    else {
        add_file_option(ag_strdup(option)); /* simple option */
    }
}

/* Prerequisite:  "option" is allocated on the heap. */
static void
add_file_option(char *option)
{
    ao->options_len++;
    ao->options = ag_realloc(ao->options, ao->options_len * sizeof(char*));
    ao->options[ao->options_len - 1] = option;
}

static char **
update_argv(int *argc, char **argv)
{
    int i, j;

    if (ao->options_len == 0) {
        return (argv);         /* empty config file */
    }
    int old_argc = *argc;
    ao->new_argv = ag_malloc((old_argc + 1 + ao->options_len) * sizeof(char*));

    j = 0;
    ao->new_argv[j++] = argv[0];   /* program name */
    for (i = 0; i < ao->options_len; i++) {
        ao->new_argv[j++] = ao->options[i];
    }
    for (i = 1; i < old_argc; i++) {
        ao->new_argv[j++] = argv[i];
    }
    ao->new_argv[j] = NULL;
    *argc = old_argc + ao->options_len;
    return (ao->new_argv);
}

void 
ag_rc_cleanup_options(void)
{
    int i;

    if (ao) {     /* be paranoid */
        if (ao->options) {
            for (i = 0; i < ao->options_len; i++) {
                (void) free(ao->options[i]);
            }
            (void) free(ao->options);
        }
        if (ao->new_argv) {
            (void) free(ao->new_argv);
        }
        (void) free(ao);
    }
}
