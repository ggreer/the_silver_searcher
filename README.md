# The Silver Searcher

A code searching tool similar to `ack`, with a focus on speed.

[![Build Status](https://travis-ci.org/ggreer/the_silver_searcher.svg?branch=master)](https://travis-ci.org/ggreer/the_silver_searcher)

[![Floobits Status](https://floobits.com/ggreer/ag.svg)](https://floobits.com/ggreer/ag/redirect)

[![#ag on Freenode](https://img.shields.io/badge/Freenode-%23ag-brightgreen.svg)](https://webchat.freenode.net/?channels=ag)

Do you know C? Want to improve ag? [I invite you to pair with me](http://geoff.greer.fm/2014/10/13/help-me-get-to-ag-10/).


## What's so great about Ag?

* It is an order of magnitude faster than `ack`.
* It ignores file patterns from your `.gitignore` and `.hgignore`.
* If there are files in your source repo you don't want to search, just add their patterns to a `.ignore` file. (\*cough\* `*.min.js` \*cough\*)
* The command name is 33% shorter than `ack`, and all keys are on the home row!

Ag is quite stable now. Most changes are new features, minor bug fixes, or performance improvements. It's much faster than Ack in my benchmarks:

    ack test_blah ~/code/  104.66s user 4.82s system 99% cpu 1:50.03 total

    ag test_blah ~/code/  4.67s user 4.58s system 286% cpu 3.227 total

Ack and Ag found the same results, but Ag was 34x faster (3.2 seconds vs 110 seconds). My `~/code` directory is about 8GB. Thanks to git/hg/ignore, Ag only searched 700MB of that.

There are also [graphs of performance across releases](http://geoff.greer.fm/ag/speed/).

## How is it so fast?

* Ag uses [Pthreads](https://en.wikipedia.org/wiki/POSIX_Threads) to take advantage of multiple CPU cores and search files in parallel.
* Files are `mmap()`ed instead of read into a buffer.
* Literal string searching uses [Boyer-Moore strstr](https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm).
* Regex searching uses [PCRE's JIT compiler](http://sljit.sourceforge.net/pcre.html) (if Ag is built with PCRE >=8.21).
* Ag calls `pcre_study()` before executing the same regex on every file.
* Instead of calling `fnmatch()` on every pattern in your ignore files, non-regex patterns are loaded into arrays and binary searched.

I've written several blog posts showing how I've improved performance. These include how I [added pthreads](http://geoff.greer.fm/2012/09/07/the-silver-searcher-adding-pthreads/), [wrote my own `scandir()`](http://geoff.greer.fm/2012/09/03/profiling-ag-writing-my-own-scandir/), [benchmarked every revision to find performance regressions](http://geoff.greer.fm/2012/08/25/the-silver-searcher-benchmarking-revisions/), and profiled with [gprof](http://geoff.greer.fm/2012/02/08/profiling-with-gprof/) and [Valgrind](http://geoff.greer.fm/2012/01/23/making-programs-faster-profiling/).


## Installing

### macOS

    brew install the_silver_searcher

or

    port install the_silver_searcher


### Linux

* Ubuntu >= 13.10 (Saucy) or Debian >= 8 (Jessie)

        apt-get install silversearcher-ag
* Fedora 21 and lower

        yum install the_silver_searcher
* Fedora 22+

        dnf install the_silver_searcher
* RHEL7+

        yum install epel-release.noarch the_silver_searcher
* Gentoo

        emerge -a sys-apps/the_silver_searcher
* Arch

        pacman -S the_silver_searcher

* Slackware

        sbopkg -i the_silver_searcher

* openSUSE:

        zypper install the_silver_searcher

* CentOS:
        
        yum install the_silver_searcher

* SUSE Linux Enterprise: Follow [these simple instructions](https://software.opensuse.org/download.html?project=utilities&package=the_silver_searcher).


### BSD

* FreeBSD

        pkg install the_silver_searcher
* OpenBSD/NetBSD

        pkg_add the_silver_searcher

### Cygwin

Run the relevant [`setup-*.exe`](https://cygwin.com/install.html), and select "the\_silver\_searcher" in the "Utils" category.

### Windows

Unofficial daily builds are [available](https://github.com/k-takata/the_silver_searcher-win32).

## Building from source

### Building master

1. Install dependencies (Automake, pkg-config, PCRE, LZMA):
    * macOS:

            brew install automake pkg-config pcre xz
        or

            port install automake pkgconfig pcre xz
    * Ubuntu/Debian:

            apt-get install -y automake pkg-config libpcre3-dev zlib1g-dev liblzma-dev
    * Fedora:

            yum -y install pkgconfig automake gcc zlib-devel pcre-devel xz-devel
    * CentOS:

            yum -y groupinstall "Development Tools"
            yum -y install pcre-devel xz-devel
    * openSUSE:

            zypper source-install --build-deps-only the_silver_searcher

    * Windows: It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).
2. Run the build script (which just runs aclocal, automake, etc):

        ./build.sh

  On Windows (inside an msys/MinGW shell):

        make -f Makefile.w32
3. Make install:

        sudo make install


### Building a release tarball

GPG-signed releases are available [here](http://geoff.greer.fm/ag).

Building release tarballs requires the same dependencies, except for automake and pkg-config. Once you've installed the dependencies, just run:

    ./configure
    make
    make install

You may need to use `sudo` or run as root for the make install.


## Editor Integration

### Vim

You can use Ag with [ack.vim][] by adding the following line to your `.vimrc`:

    let g:ackprg = 'ag --nogroup --nocolor --column'

or:

    let g:ackprg = 'ag --vimgrep'

Which has the same effect but will report every match on the line.

### Emacs

You can use [ag.el][] as an Emacs front-end to Ag. See also: [helm-ag].

[ag.el]: https://github.com/Wilfred/ag.el
[helm-ag]: https://github.com/syohex/emacs-helm-ag

### TextMate

TextMate users can use Ag with [my fork](https://github.com/ggreer/AckMate) of the popular AckMate plugin, which lets you use both Ack and Ag for searching. If you already have AckMate you just want to replace Ack with Ag, move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`

## Other stuff you might like

* [Ack](https://github.com/petdance/ack2) - Better than grep. Without Ack, Ag would not exist.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://git-scm.com/docs/git-grep) - As fast as Ag but only works on git repos.
* [ripgrep](https://github.com/BurntSushi/ripgrep)
* [Sack](https://github.com/sampson-chen/sack) - A utility that wraps Ack and Ag. It removes a lot of repetition from searching and opening matching files.
