# The Silver Searcher #

An attempt to make something better than ack, which itself is better than grep.

## Why use Ag? ##

* It searches through code about 3x-5x faster than Ack.
* It ignores files matched by patterns in your .gitignore and .hgignore.
* If there are files in your source repo that you don't want to search, just add their patterns to a .agignore file. \*cough\* extern \*cough\*
* The binary name is 33% shorter than Ack!

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

If you get the error `-bash: /usr/bin/ag: No such file or directory`, congratulations, you are one of the lucky few. Apt managed to install the i386 version of Ag on an x86-64 system. I have no idea why this happens. To fix this issue, you'll have to `apt-get install ia32-libs` or `apt-get purge the-silver-searcher` and build from source.

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

It's quite stable now. Most changes are new features, minor bug fixes, or performance improvements. It's much faster than Ack in my benchmarks.

    ack blahblahblah ~/code  6.59s user 1.94s system 99% cpu 8.547 total

    ag blahblahblah ~/code  1.39s user 1.81s system 229% cpu 1.396 total

You can use Ag with [my fork](https://github.com/ggreer/AckMate) of the popular ackmate plugin, which lets you use both Ack and Ag for searching in Textmate. You can also move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`

## TODO ##
A special thanks goes out to Alex Davies. He has given me some excellent recommendations to improve Ag. Many of these things are still on my list:

* Optimizations
  * Profile `read()` against `mmap()`. Remember that's `read()` not `fread()`.
  * Write a benchmarking script that tweaks various settings to find what's fastest.
* Features
  * Symlink loop detection.
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
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://git-scm.com/docs/git-grep) - As fast as Ag but only works on git repos.
