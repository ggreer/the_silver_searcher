AC_INIT([the_silver_searcher], [0.6], [https://github.com/ggreer/the_silver_searcher/issues], [the_silver_searcher], [https://github.com/ggreer/the_silver_searcher])
AC_PROG_CC
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([no-define foreign subdir-objects])

m4_ifdef(
    [AM_SILENT_RULES],
    [AM_SILENT_RULES([yes])]
)

PKG_CHECK_MODULES([PCRE], [libpcre])

CFLAGS="$CFLAGS $PCRE_CFLAGS -Wall -Wextra -std=c89 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -g"
LDFLAGS="$LDFLAGS"

AC_CHECK_DECL([PCRE_CONFIG_JIT], [AC_DEFINE([USE_PCRE_JIT], [], [Use PCRE JIT])], [], [#include <pcre.h>])

AC_CHECK_FUNCS(getline strlcpy strlcat strndup)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT
