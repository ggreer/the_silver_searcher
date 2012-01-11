AC_INIT([the_silver_searcher], [0.1])
AC_PROG_CC
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([no-define foreign])

AC_CHECK_HEADERS([pcre.h], [], [AC_MSG_ERROR("pcre.h not found")],[])
AC_CHECK_LIB([pcre], [pcre_exec])

AC_CHECK_DECL([PCRE_CONFIG_JIT], [AC_DEFINE([USE_PRCE_JIT], [], [Use PCRE JIT])], [], [#include <pcre.h>])

AC_DEFINE([AG_VERSION], ["0.1.1"], [Ag Version])
CFLAGS="$CFLAGS -Wall -Wextra -std=c89 -D_GNU_SOURCE -g"
LDFLAGS="$LDFLAGS"

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT
