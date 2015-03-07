ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/ignore.h src/log.c src/log.h src/options.c src/win32/win_colors.c src/options.h src/print.c src/print.h src/scandir.c src/scandir.h src/search.c src/search.h src/lang.c src/lang.h src/util.c src/util.h src/decompress.c src/decompress.h src/uthash.h src/main.c

# When cross-compiling with MXE, Makefile.w32 is not used but ag builds fine,
# however, to support windows native colors we need src/win32/win_colors.c
# so it's listed here as well, but is a no-op on anything except windows.

ag_LDADD = ${PCRE_LIBS} ${LZMA_LIBS} ${ZLIB_LIBS} $(PTHREAD_LIBS)

dist_man_MANS = doc/ag.1

bashcompdir = $(pkgdatadir)/completions
dist_bashcomp_DATA = ag.bashcomp.sh

EXTRA_DIST = Makefile.w32 LICENSE NOTICE the_silver_searcher.spec README.md

test: ag
	cram -v tests/*.t

test_big: ag
	cram -v tests/big/*.t

.PHONY : all test clean
