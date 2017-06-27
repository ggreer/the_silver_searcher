ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/ignore.h src/log.c src/log.h src/options.c src/options.h src/print.c src/print_w32.c src/print.h src/scandir.c src/scandir.h src/search.c src/search.h src/lang.c src/lang.h src/util.c src/util.h src/decompress.c src/decompress.h src/uthash.h src/main.c src/zfile.c
ag_LDADD = ${PCRE_LIBS} ${LZMA_LIBS} ${ZLIB_LIBS} $(PTHREAD_LIBS)

dist_man_MANS = doc/ag.1

bashcompdir = $(pkgdatadir)/completions
dist_bashcomp_DATA = ag.bashcomp.sh
zshcompdir = $(datadir)/zsh/site-functions
dist_zshcomp_DATA = _the_silver_searcher

EXTRA_DIST = Makefile.w32 LICENSE NOTICE the_silver_searcher.spec README.md

all:
	@$(MAKE) ag -r

test: ag
	cram -v tests/*.t
if HAS_CLANG_FORMAT
	CLANG_FORMAT=${CLANG_FORMAT} ./format.sh test
else
	@echo "clang-format is not available. Skipped clang-format test."
endif

test_big: ag
	cram -v tests/big/*.t

test_fail: ag
	cram -v tests/fail/*.t

.PHONY : all clean test test_big test_fail
