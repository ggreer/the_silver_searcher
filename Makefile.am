ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/scandir.c src/search.c src/util.c src/decompress.c src/main.c src/lang.c
ag_LDADD = ${PCRE_LIBS} ${LZMA_LIBS} -lz -lpthread

man_MANS = doc/ag.1

bashcompdir = $(pkgdatadir)/completions
bashcomp_DATA = ag.bashcomp.sh

# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg

EXTRA_DIST = $(ASSEMBLY_NAME).spec.in
