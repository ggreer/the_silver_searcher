# The Silver Searcher

A code searching tool similar to `ack`, with a focus on speed.

[![Build Status](https://travis-ci.org/ggreer/the_silver_searcher.svg?branch=master)](https://travis-ci.org/ggreer/the_silver_searcher)

<a href="https://floobits.com/ggreer/ag/redirect">
  <img alt="Floobits status" width="100" height="40" src="https://floobits.com/ggreer/ag.png" />
</a>


## What's so great about Ag?

* It is an order of magnitude faster than `ack`.
* It ignores file patterns from your `.gitignore` and `.hgignore`.
* If there are files in your source repo you don't want to search, just add their patterns to a `.agignore` file. (\*cough\* extern \*cough\*)
* The command name is 33% shorter than `ack`, and all keys are on the home row!

Ag is quite stable now. Most changes are new features, minor bug fixes, or performance improvements. It's much faster than Ack in my benchmarks:

    ack test_blah ~/code/  104.66s user 4.82s system 99% cpu 1:50.03 total

    ag test_blah ~/code/  4.67s user 4.58s system 286% cpu 3.227 total

Ack and Ag found the same results, but Ag was 34x faster (3.2 seconds vs 110 seconds). My `~/code` directory is about 8GB, though thanks to git/hg/svn-ignore, Ag only searched 700MB of that.

You can also take a look at [graphs of performance across releases](http://geoff.greer.fm/ag/speed/).

## How is it so fast?

* Searching for literals (no regex) uses [Boyer-Moore strstr](https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm).
* Files are `mmap()`ed instead of read into a buffer.
* If you're building with PCRE 8.21 or greater, regex searches use [the JIT compiler](http://sljit.sourceforge.net/pcre.html).
* Ag calls `pcre_study()` before executing the regex on a jillion files.
* Instead of calling `fnmatch()` on every pattern in your ignore files, non-regex patterns are loaded into an array and binary searched.
* Ag uses [Pthreads](https://en.wikipedia.org/wiki/POSIX_Threads) to take advantage of multiple CPU cores and search files in parallel.

I've written several blog posts showing how I've improved performance. These include how I [added pthreads](http://geoff.greer.fm/2012/09/07/the-silver-searcher-adding-pthreads/), [wrote my own `scandir()`](http://geoff.greer.fm/2012/09/03/profiling-ag-writing-my-own-scandir/), [benchmarked every revision to find performance regressions](http://geoff.greer.fm/2012/08/25/the-silver-searcher-benchmarking-revisions/), and profiled with [gprof](http://geoff.greer.fm/2012/02/08/profiling-with-gprof/) and [Valgrind](http://geoff.greer.fm/2012/01/23/making-programs-faster-profiling/).


## Installing

### OS X

    brew install the_silver_searcher

or

    port install the_silver_searcher


### Linux

* Ubuntu >= 13.10 (Saucy) or Debian >= 8 (Jessie)

        apt-get install silversearcher-ag
* Fedora 19+

        yum install the_silver_searcher
* RHEL7+

        rpm -Uvh http://dl.fedoraproject.org/pub/epel/beta/7/x86_64/epel-release-7-0.2.noarch.rpm
        yum install the_silver_searcher
* Gentoo

        emerge the_silver_searcher
* Arch

        pacman -S the_silver_searcher


### BSD

* FreeBSD

        pkg install the_silver_searcher
* OpenBSD/NetBSD

        pkg_add the_silver_searcher


### Fedora

    yum install the_silver_searcher

## Building from source

1. Install dependencies (Automake, pkg-config, PCRE, LZMA):
    * OS X:

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
    * Windows: It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).
2. Run the build script (which just runs aclocal, automake, etc):

        ./build.sh

  On Windows:

        mingw32-make -f Makefile.w32
3. Make install:

        sudo make install


## Editor Integration

### Vim

You can use Ag with [ack.vim][] by adding the following line to your `.vimrc`:

    let g:ackprg = 'ag --nogroup --nocolor --column'

or:

    let g:ackprg = 'ag --vimgrep'

Which has the same effect but will report every match on the line.

There's also a fork of ack.vim tailored for use with Ag: [ag.vim][]
[ack.vim]: https://github.com/mileszs/ack.vim
[ag.vim]: https://github.com/rking/ag.vim

### Emacs

You can use use [ag.el][] as an Emacs fronted to Ag.

[ag.el]: https://github.com/Wilfred/ag.el

### TextMate

TextMate users can use Ag with [my fork](https://github.com/ggreer/AckMate) of the popular AckMate plugin, which lets you use both Ack and Ag for searching. If you already have AckMate you just want to replace Ack with Ag, move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`


## Contributing

I like when people send pull requests. It validates my existence. If you want to help out, check the [issue list](https://github.com/ggreer/the_silver_searcher/issues?sort=updated&state=open) or search the codebase for `TODO`. Don't worry if you lack experience writing C. If I think a pull request isn't ready to be merged, I'll give feedback in comments. Once everything looks good, I'll comment on your pull request with a cool animated gif and hit the merge button.


## Other stuff you might like

* [Ack](https://github.com/petdance/ack) - Better than grep. Without Ack, Ag would not exist.
* [AckMate](https://github.com/protocool/AckMate) - An ack-powered replacement for TextMate's slow built-in search.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [ag.vim]( https://github.com/rking/ag.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://git-scm.com/docs/git-grep) - As fast as Ag but only works on git repos.
* [Sack](https://github.com/sampson-chen/sack) - A utility that wraps Ack and Ag. It removes a lot of repetition from searching and opening matching files.
