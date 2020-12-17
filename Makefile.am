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

#########################################################################################

RUST_SOURCE =\
	rust/Cargo.toml\
	rust/src/lib.rs

RUST_EXTRA =\
	rust/Cargo.lock

if DEBUG_RELEASE
CARGO_RELEASE_ARGS=
else
CARGO_RELEASE_ARGS=--release
endif

rust_dir = $(top_srcdir)/rust

check-local-rust:
	cd $(rust_dir) && \
	CARGO_TARGET_DIR=target cargo test

clean-local-rust:
	cd $(rust_dir) && \
	cargo clean

RUST_LIB=$(rust_dir)/target/@RUST_TARGET_SUBDIR@/librust_internals.a

$(RUST_LIB): $(RUST_SOURCE)
	cd $(rust_dir) && \
	CARGO_TARGET_DIR=target cargo build --verbose $(CARGO_RELEASE_ARGS)

EXTRA_DIST += \
	$(RUST_SOURCE) \
	$(RUST_EXTRA)

ag_LDADD += $(RUST_LIB)

# solve undefined reference to 'dlsym' caused by newer
# versions of gcc/ld default to linking with --as-needed.
ag_LDFLAGS = -Wl,--no-as-needed -ldl

#########################################################################################

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
