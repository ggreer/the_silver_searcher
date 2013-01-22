ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/scandir.c src/search.c src/util.c src/main.c
ag_LDADD = ${PCRE_LIBS} -lpthread

man_MANS = doc/ag.1

bashcompdir = $(sysconf)/bash_completion.d
bashcomp_DATA = ag.bashcomp.sh

# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg
