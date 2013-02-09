ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/scandir.c src/search.c src/util.c src/decompress.c src/main.c
ag_LDADD = ${PCRE_LIBS} -lz -lpthread

man_MANS = doc/ag.1

bashcompdir = $(pkgdatadir)/completions
bashcomp_DATA = ag.bashcomp.sh

# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg
