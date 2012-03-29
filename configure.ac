AC_INIT([the_silver_searcher], [0.5])
AC_PROG_CC
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([no-define foreign subdir-objects])

m4_ifdef(
    [AM_SILENT_RULES],
    [AM_SILENT_RULES([yes])]
)

PKG_CHECK_MODULES([PCRE], [libpcre])

CFLAGS="$CFLAGS $PCRE_CFLAGS -Wall -Wextra -std=c89 -D_GNU_SOURCE -g"
LDFLAGS="$LDFLAGS"

AC_CHECK_DECL([PCRE_CONFIG_JIT], [AC_DEFINE([USE_PCRE_JIT], [], [Use PCRE JIT])], [], [#include <pcre.h>])

# the kqueue part of this is slightly ugly, but it's one cheap way to get BSD detection
AC_CHECK_FUNCS(getline strlcpy strlcat strndup kqueue)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT
