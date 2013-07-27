ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/scandir.c src/search.c src/util.c src/decompress.c src/main.c
ag_LDADD = ${PCRE_LIBS} ${LZMA_LIBS} -lz -lpthread

man_MANS = doc/ag.1

bashcompdir = $(pkgdatadir)/completions
dist_bashcomp_DATA = ag.bashcomp.sh
sysbashcompdir = `pkg-config --variable=completionsdir bash-completion`
sysbashcompscript = $(sysbashcompdir)/ag
completion:
	-$(LN_S) $(bashcompdir)/ag.bashcomp.sh $(sysbashcompscript)
# Uncomment this line to output gprof profiling info. This hurts performance.
#CFLAGS=-pg

install-data-hook: completion

uninstall-local:
	-$(RM) $(sysbashcompscript)

