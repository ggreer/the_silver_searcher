ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/ignore.h src/log.c src/log.h src/options.c src/options.h src/print.c src/print.h src/scandir.c src/scandir.h src/search.c src/search.h src/lang.c src/lang.h src/util.c src/util.h src/decompress.c src/decompress.h src/uthash.h src/main.c
ag_LDADD = ${PCRE_LIBS} ${LZMA_LIBS} -lz -lpthread

dist_man_MANS = doc/ag.1

${builddir}/completion/ag: ${srcdir}/completion/ag.bashcomp.sh
	mkdir -p ${builddir}/completion
	sed 's/.*have ag.*/# Generated from ag.bashcomp.sh. Please do not edit or add to repository/' ${srcdir}/completion/ag.bashcomp.sh > $@
bashcompdir = @bashcompdir@
bashcomp_DATA = @bashcompdata@

# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg

test:
	cram -v tests/*.t

EXTRA_DIST = Makefile.w32 LICENSE NOTICE the_silver_searcher.spec README.md completion/ag.bashcomp.sh
