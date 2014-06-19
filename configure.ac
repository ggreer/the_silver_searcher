AC_INIT(
    [the_silver_searcher],
    [0.23.0],
    [https://github.com/ggreer/the_silver_searcher/issues],
    [the_silver_searcher],
    [https://github.com/ggreer/the_silver_searcher])

AM_INIT_AUTOMAKE([no-define foreign subdir-objects])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PREREQ([2.59])

m4_ifdef(
    [AM_SILENT_RULES],
    [AM_SILENT_RULES([yes])])

PKG_CHECK_MODULES([PCRE], [libpcre])

# Run CFLAGS="-g" ./configure if you want debug symbols
CFLAGS="$CFLAGS $PCRE_CFLAGS -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces -std=gnu89 -D_GNU_SOURCE -O2"
LDFLAGS="$LDFLAGS"

AC_CHECK_HEADERS([pthread.h])

AC_ARG_ENABLE([zlib],
    AS_HELP_STRING([--disable-zlib], [Disable zlib compressed search support]))

AS_IF([test "x$enable_zlib" != "xno"], [
    AC_CHECK_HEADERS([zlib.h])
    AC_SEARCH_LIBS([inflate], [zlib, z])
])

AC_ARG_ENABLE([lzma],
    AS_HELP_STRING([--disable-lzma], [Disable lzma compressed search support]))

AS_IF([test "x$enable_lzma" != "xno"], [
    AC_CHECK_HEADERS([lzma.h])
    PKG_CHECK_MODULES([LZMA], [liblzma])
])

AC_CHECK_DECL([PCRE_CONFIG_JIT], [AC_DEFINE([USE_PCRE_JIT], [], [Use PCRE JIT])], [], [#include <pcre.h>])

AC_CHECK_MEMBER([struct dirent.d_type], [AC_DEFINE([HAVE_DIRENT_DTYPE], [], [Have dirent struct member d_type])], [], [[#include <dirent.h>]])

AC_CHECK_FUNCS(fgetln getline realpath strlcpy strndup vasprintf)

AC_CONFIG_FILES([Makefile the_silver_searcher.spec])
AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT
