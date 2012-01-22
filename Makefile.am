ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/util.c src/main.c
ag_LDADD = ${PCRE_LIBS}

man_MANS = doc/ag.1

# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg
