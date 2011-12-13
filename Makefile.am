AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

CFLAGS = -Wall -std=c99 -D_GNU_SOURCE -O2 -g
LDFLAGS = -lpcre

bin_PROGRAMS = ag
ag_SOURCES = src/ignore.c src/log.c src/options.c src/print.c src/main.c
