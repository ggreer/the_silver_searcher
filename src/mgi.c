/*
 * Mimic Git Ignore
 *
 * consume the output of `git status --ignored --porcelain` and
 * load into Ag's "ignore" data structure.  Idea suggested by
 * Jason Karns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "util.h"
#include "log.h"
#include "ignore.h"
#include "mgi.h"

static void note_unexpected_git_tl_wd_issue(const char *, const char *);

void
autoload_git_ignored_files(void)
{
    int i, cwd_ok;
    FILE *popen_fp = NULL;
    size_t blen = PATH_MAX;
    char *cwd = ag_malloc(blen);
    char *strip_prefix = NULL;
    size_t strip_len = 0;

    /* need to fetch cwd for path analysis */
    for (i = 0, cwd_ok = FALSE; (i < 4) && (!cwd_ok); i++) {
        if (getcwd(cwd, blen) == NULL) {
            blen *= 2;
            cwd = ag_realloc(cwd, blen);
        } else {
            cwd_ok = TRUE;
        }
    }
    if (! cwd_ok) {
        /* something not right here... */

        log_err("--mgi: getcwd() failed: %s", strerror(errno));
        (void) free(cwd);
        return;
    }

    /* next, obtain top level locn of git repository */
#ifdef _WIN32
    popen_fp = popen("git rev-parse --show-toplevel 2>NULL", "r");
#else
    popen_fp = popen("git rev-parse --show-toplevel 2>/dev/null", "r");
#endif
    if (!popen_fp) {
        log_warn("--mgi: git failed to start");
        (void) free(cwd);
        return;
    }
    size_t git_tl_wd_len = 0;
    char *git_tl_wd = NULL;
    do {
        git_tl_wd = ag_realloc(git_tl_wd, git_tl_wd_len + 1024);
        git_tl_wd_len += fread(git_tl_wd + git_tl_wd_len, 1, 1023, popen_fp);
    } while (!feof(popen_fp) && git_tl_wd_len > 0 && git_tl_wd_len % 1023 == 0);
    git_tl_wd[git_tl_wd_len] = '\0';
    (void) pclose(popen_fp);

    if (git_tl_wd_len > 0 && git_tl_wd[git_tl_wd_len - 1] == '\n') {
        git_tl_wd[--git_tl_wd_len] = '\0';
    }

    /*
     * In a cygwin env (at the least), popen() will not return NULL if
     * git is not reachable via $PATH and the returned file pointer will
     * not indicate EOF either.  Instead, the first buffer read returns
     * a single NUL char.  Whatever.  Of course, if cwd is _not_ a git
     * repository, then we don't worry about toplevel issues.  But soldier
     * on (because "git status --ignored --porcelain" could list
     * _global_ file patterns that should be ignored).
     *
     * Leave some debug info, for users.
     */
    log_debug("git rev-parse --show-toplevel: %s" ,
              (git_tl_wd[0] == '\0') ? "<nothing>" : git_tl_wd);

    /*
     * Now, decide if paths returned by "git status --ignored" need to be
     * adjusted for a difference in the user's cwd.
     */
    if (git_tl_wd_len > 0 && strcmp(git_tl_wd, cwd) != 0) {
        size_t cwd_len = strlen(cwd);
        if (cwd_len > git_tl_wd_len) {
            /*
             * Current situation looks something like this:
             *     git tl wd:  /cygdrive/c/git/the_silver_searcher
             *     cwd:        /cygdrive/c/git/the_silver_searcher/src
             *
             * In this situation, we want to strip "src/" off every relative
             * path returned by "git status".  Reason, git status returns
             * ignored repository paths like so (based on its TL WD):
             *
             *    src/options.o
             *    src/print.o
             *    src/print_w32.o
             *    src/scandir.o
             *      etc.
             */

            if (strncmp(cwd, git_tl_wd, git_tl_wd_len) == 0) {
                strip_prefix = cwd + git_tl_wd_len;
                if (*strip_prefix == '/') {
                    strip_prefix++;
                }
                strip_len = strlen(strip_prefix);
                log_debug("--mgi: strip_prefix: %s", strip_prefix);
            } else {
                /* git TL WD not a substr of cwd(). should not happen... */

                note_unexpected_git_tl_wd_issue(git_tl_wd, cwd);
            }
        } else {
            /* 
             * Len(cwd) <= len(git TL WD) and the paths don't match. This
             * should not happen...
             */

            note_unexpected_git_tl_wd_issue(git_tl_wd, cwd);
        }
    }

#ifdef _WIN32
    popen_fp = popen("git status --ignored --porcelain 2>NULL", "r");
#else
    popen_fp = popen("git status --ignored --porcelain 2>/dev/null", "r");
#endif

    if (!popen_fp) {
        (void) free(git_tl_wd);
        (void) free(cwd);
        log_warn("--mgi: git failed to start");
        return;
    }

    size_t rawbuf_len = 0;
    char *rawbuf = NULL;
    do {
        rawbuf = ag_realloc(rawbuf, rawbuf_len + 1024);
        rawbuf_len += fread(rawbuf + rawbuf_len, 1, 1023, popen_fp);
    } while (!feof(popen_fp) && rawbuf_len > 0 && rawbuf_len % 1023 == 0);
    rawbuf[rawbuf_len] = '\0';
    (void) pclose(popen_fp);

    /*
     * In a cygwin env (at the least), popen() will not return NULL if
     * git is not reachable via $PATH and the returned file pointer will
     * not indicate EOF either.  Instead, the first buffer read returns
     * a single NUL char.  Whatever.
     *
     * And then there's the use case where "git --ignored --porcelain"
     * returns nothing...and that actually happens in some clean repos.
     *
     * Leave a debug clue for the user.
     */
    if (rawbuf[0] == '\0') {
        log_debug("git --ignored --porcelain returned no data.");
        (void) free(rawbuf);
        (void) free(git_tl_wd);
        (void) free(cwd);
        return;
    }

    /*
     * git's listing format of ignored files is:
     *
     * !!<space><file_path>
     *
     * Adjust the returned listing data as follows:
     *
     * 1) remove listing format chars from first 3 columns,
     * 2) drop files that don't meet the ignored criteria. E.g., drop
     *    untracked files from porcelain listing (ex:  "?? junk/").
     * 3) adjust for differences in cwd and git TL WD.
     */
    char *gitpath = ag_malloc(PATH_MAX);
    char *cp = rawbuf;
    while (*cp)
    {
        if (*cp == '\n') {
            cp++;
        } else if (cp[0] == '!' && cp[1] == '!' && cp[2] == ' ') {
            cp += 3;
            i = 0;
            while (*cp && (*cp != '\n') && (i < PATH_MAX - 2)) {
                gitpath[i++] = *cp++;
            }
            gitpath[i] = '\0';
            char *gp = gitpath;
            if (strncmp(gp, strip_prefix, strip_len) == 0) {
                gp += strip_len;
                if (*gp == '/') {
                    gp++;
                }
            }
            if (*gp) {
                add_ignore_pattern(root_ignores, gp);
            }
        } else {
            do {
                cp++;
            } while (*cp && *cp != '\n');
        }
    }
    (void) free(gitpath);
    (void) free(rawbuf);
    (void) free(git_tl_wd);
    (void) free(cwd);
}

/*
 * FIXME -- log_warn() => log_debug() after code has been exercised for a
 * longish period of time.
 */
static void
note_unexpected_git_tl_wd_issue(const char *git_tl_wd, const char *cwd)
{
    log_warn("--mgi: git top level differs from cwd...(unexpected case)!");
    log_warn("--mgi: git top level wd: %s", git_tl_wd);
    log_warn("--mgi: cwd: %s", cwd);
}
