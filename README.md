# The Silver Searcher #

An attempt to make something better than ack, which itself is better than grep.

## Why use ag? ##

* It searches through code about 3x-5x faster than ack.
* It ignores files matched by patterns in your .gitignore and .hgignore.
* If there are files in your source repo that you don't want to search, just add their patterns to a .agignore file. \*cough\* extern \*cough\*
* The binary name is 33% shorter than ack!

## How is it so fast? ##

* Searching for literals (no regex) uses [Boyer-Moore-Horspool strstr](http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm).
* Files are mmap()ed instead of read into a buffer.
* If you're building with PCRE 8.21 or greater, regex searches use [the JIT compiler](http://sljit.sourceforge.net/pcre.html).
* Ag calls pcre_study() before executing the regex on a jillion files.
* Instead of calling fnmatch() on every pattern in your ignore files, non-regex patterns are loaded into an array and binary searched.

[This blog post](http://geoff.greer.fm/2012/01/23/making-programs-faster-profiling/) gives an idea of how I go about improving performance.

## Installation ##

Ubuntu 12.04+:

[Add the PPA](https://help.ubuntu.com/community/Repositories/Ubuntu#Adding_PPAs). Make sure you have `python-software-properties` installed, then:

    sudo add-apt-repository ppa:ggreer/ag
    sudo apt-get update
    sudo apt-get install the-silver-searcher

If you get the error `-bash: /usr/bin/ag: No such file or directory`, congratulations, apt managed to install the i386 version on an x86-64 system. This is Launchpad's fault. It sometimes doesn't build 64-bit versions of packages. To fix this issue, you'll have to `apt-get install ia32-libs` or `apt-get purge the-silver-searcher` and build from source.

Gentoo:

    emerge the_silver_searcher

OS X:

    brew install the_silver_searcher

For debs, rpms, and static builds, see the [downloads page](https://github.com/ggreer/the_silver_searcher/downloads).

## Building from source ##

1. Install PCRE development libraries:
    * Ubuntu: `apt-get install -y libpcre3-dev`
    * OS X: Install [homebrew](http://mxcl.github.com/homebrew/), then `brew install pcre`
    * Windows: It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).
2. Run the build script (which just runs aclocal, automake, etc):
    * `./build.sh`
3. Make install:
    * `sudo make install`

## Current development status ##

It's pretty stable now. Most changes are new features or minor bug fixes, such as support for named pipes. It's much faster than ack in my benchmarks.

    ack -i SOLR ~/cloudkick/reach  2.89s user 0.77s system 97% cpu 3.765 total

    ag -i SOLR ~/cloudkick/reach  0.25s user 0.13s system 94% cpu 0.404 total

You can use ag with [my fork](https://github.com/ggreer/AckMate) of the popular ackmate plugin, which lets you use both ack and ag for searching in Textmate. You can also move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`

## TODO ##
* behave better when matching in files with really long lines
  * maybe say "match found at position X of line N" if line is > 10k chars
* optimizations
  * pthreads. these days it's silly to use only one core
* actually get textmate dir patterns working (this requires ruby regexes. not fun)
* symlink loop detection

## Other stuff you might like ##
* [Ack](https://github.com/petdance/ack) - Better than grep
* [AckMate](https://github.com/protocool/AckMate) - An ack-powered replacement for TextMate's slow built-in search.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://book.git-scm.com/4_finding_with_git_grep.html) - As fast as Ag but only works on git repos.
