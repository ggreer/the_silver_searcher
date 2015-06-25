#ifndef AG_RC_H
#define AG_RC_H

struct ag_rc_options {
    char **new_argv;
    char **options;
    int  options_len;
    int  list_argv;
};
typedef struct ag_rc_options ag_rc_options;

extern char **ag_rc_read_options(int *argc, char **argv, int *list_argv);
extern void ag_rc_cleanup_options(void);

#endif  /* AG_RC_H */
