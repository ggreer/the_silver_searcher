# The Silver Searcher #

A code searching tool similar to `ack`, with a focus on speed.

<a href="https://floobits.com/r/ggreer/the_silver_searcher/redirect">
  <img alt="Floobits status" width="80" height="40" src="https://floobits.com/r/ggreer/the_silver_searcher.png" />
</a>


## What's so great about Ag? ##

* It searches code about 3–5× faster than `ack`.
* It ignores file patterns from your `.gitignore` and `.hgignore`.
* If there are files in your source repo you don't want to search, just add their patterns to a `.agignore` file. \*cough\* extern \*cough\*
* The command name is 33% shorter than `ack`, and all keys are on the home row!

## Common Usage ##

Search across application files for a phrase:

    ag my-search-phrase

Search for files *not* containing a phrase:

    ag -v ignore-this-phrase

Search only Python files:

    ag --python my-search-phrase

This trick works for many common languages and filetypes: [a full list can be
found here][filetypes].

 [filetypes]: https://github.com/ggreer/the_silver_searcher/blob/master/src/lang.c#L7

Print 5 lines of context before and after a match:

    ag -C 5 my-search-phrase
    # (use -B <num> for before, -A <num> for lines after a match)

## How is it so fast? ##

* Searching for literals (no regex) uses [Boyer-Moore-Horspool strstr](http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm).
* Files are `mmap()`ed instead of read into a buffer.
* If you're building with PCRE 8.21 or greater, regex searches use [the JIT compiler](http://sljit.sourceforge.net/pcre.html).
* Ag calls `pcre_study()` before executing the regex on a jillion files.
* Instead of calling `fnmatch()` on every pattern in your ignore files, non-regex patterns are loaded into an array and binary searched.
* Ag uses [Pthreads](http://en.wikipedia.org/wiki/POSIX_Threads) to take advantage of multiple CPU cores and search files in parallel.

I've written several blog posts showing how I've improved performance. These include how I [added pthreads](http://geoff.greer.fm/2012/09/07/the-silver-searcher-adding-pthreads/), [wrote my own `scandir()`](http://geoff.greer.fm/2012/09/03/profiling-ag-writing-my-own-scandir/), [benchmarked every revision to find performance regressions](http://geoff.greer.fm/2012/08/25/the-silver-searcher-benchmarking-revisions/), and profiled with [gprof](http://geoff.greer.fm/2012/02/08/profiling-with-gprof/) and [Valgrind](http://geoff.greer.fm/2012/01/23/making-programs-faster-profiling/).


## Installation ##

Gentoo:

    emerge the_silver_searcher

OS X:

    brew install the_silver_searcher

or

    port install the_silver_searcher

ArchLinux:

    pacman -S the_silver_searcher

FreeBSD:

Use either of the following commands depending on which [package management tool](http://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/ports.html) your system is configured for:

    pkg add the_silver_searcher

or

    pkg_add the_silver_searcher

To build Ag from source on FreeBSD:

    make -C /usr/ports/textproc/the_silver_searcher install clean

OpenBSD:

Use the following command to install from packages:

    pkg_add the_silver_searcher

To build Ag from source on OpenBSD:

    cd /usr/ports/textproc/the_silver_searcher && make install

If you want a CentOS rpm or Ubuntu deb, take a look at [Vikram Dighe's packages](http://swiftsignal.com/packages/).

Debian unstable:

    apt-get install silversearcher-ag

Ubuntu 13.10 or later:

    apt-get install silversearcher-ag

Ubuntu 13.04:

    apt-get install software-properties-common # (if required)
    apt-add-repository ppa:mizuno-as/silversearcher-ag
    apt-get update
    apt-get install silversearcher-ag

## Building from source ##

1. Install dependencies (Automake, pkg-config, PCRE, LZMA):
    * Ubuntu: `apt-get install -y automake pkg-config libpcre3-dev zlib1g-dev liblzma-dev`
    * CentOS:

              yum -y groupinstall "Development Tools"
              yum -y install pcre-devel xz-devel
    * OS X:
        - Install [homebrew](http://mxcl.github.com/homebrew/), then `brew install automake pkg-config pcre`
        - Or install [macports](http://macports.org), then `port install automake pkgconfig pcre`
    * Windows: It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).
2. Run the build script (which just runs aclocal, automake, etc):
    * `./build.sh`
3. Make install:
    * `sudo make install`

On windows:

    mingw32-make -f Makefile.w32


## Current development status ##

It's quite stable now. Most changes are new features, minor bug fixes, or performance improvements. It's much faster than Ack in my benchmarks.

    ack blahblahblah ~/code  6.59s user 1.94s system 99% cpu 8.547 total

    ag blahblahblah ~/code  1.39s user 1.81s system 229% cpu 1.396 total


## Editor Integration ##

### TextMate ###

TextMate users can use Ag with [my fork](https://github.com/ggreer/AckMate) of the popular AckMate plugin, which lets you use both Ack and Ag for searching. If you already have AckMate you just want to replace Ack with Ag, move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`

### Vim ###

You can use Ag with [ack.vim][] by adding the following line to your `.vimrc`:

    let g:ackprg = 'ag --nogroup --nocolor --column'

There's also a fork of ack.vim tailored for use with Ag: [ag.vim][]
[ack.vim]: https://github.com/mileszs/ack.vim
[ag.vim]: https://github.com/rking/ag.vim

### Emacs ###

You can use use [ag.el][] as an Emacs fronted to Ag.

[ag.el]: https://github.com/Wilfred/ag.el

## Contributing ##

I like when people send pull requests. It validates my existence. If you want to help out, check the [issue list](https://github.com/ggreer/the_silver_searcher/issues?sort=updated&state=open) or search the codebase for `TODO`. Don't worry if you lack experience writing C. If I think a pull request isn't ready to be merged, I'll give feedback in comments. Once everything looks good, I'll comment on your pull request with a cool animated gif and hit the merge button.


## TODO ##

A special thanks goes out to Alex Davies. He has given me some excellent recommendations to improve Ag. Many of these things are still on my list:

* Optimizations
  * Profile `read()` against `mmap()`. Remember that's `read()` not `fread()`.
  * Write a benchmarking script that tweaks various settings to find what's fastest.
* Features
  * Behave better when matching in files with really long lines.
  * Report "match found at position X of line N" if line is > 10k chars.
* Windows support
  * `readdir()` and `stat()` are much slower on Windows. Use `FindNextFile()` instead.
  * Support Visual Studio instead of autotools?
  * Need to use pthreads-win32 or something similar.


## Other stuff you might like ##

* [Ack](https://github.com/petdance/ack) - Better than grep
* [AckMate](https://github.com/protocool/AckMate) - An ack-powered replacement for TextMate's slow built-in search.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [ag.vim]( https://github.com/rking/ag.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://git-scm.com/docs/git-grep) - As fast as Ag but only works on git repos.
* [Sack](https://github.com/sampson-chen/sack) - A utility that wraps Ack and Ag. It removes a lot of repetition from searching and opening matching files.
