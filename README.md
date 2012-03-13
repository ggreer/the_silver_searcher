# The Silver Searcher #

An attempt to make something better than ack, which itself is better than grep.

## Why use ag? ##

* It searches through code about 3x-5x faster than ack.
* It ignores files matched by patterns in your .gitignore and .hgignore.
* If there are files in your source repo that you don't want to search, just add their patterns to a .agignore file. \*cough\* extern \*cough\*
* It's written in C instead of perl, the lesser of two evils :)
* The binary name is 33% shorter than ack!

## Installation ##
Ubuntu:

    sudo add-apt-repository ppa:ggreer/ag
    sudo apt-get install the-silver-searcher

Gentoo:

    emerge the_silver_searcher

OS X:

    brew install pcre pkg-config
Then build from source. Once Ag becomes more popular, hopefully someone will submit a pull request to homebrew.

For debs, rpms, and static builds, see the [downloads page](https://github.com/ggreer/the_silver_searcher/downloads).

### Building from source ###

1. Install PCRE development libraries:
    * Ubuntu: `apt-get install -y libpcre3-dev`
    * OS X: Install [homebrew](http://mxcl.github.com/homebrew/), then `brew install pcre`
    * Windows: It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).
2. Run the build script (which just runs aclocal, automake, etc):
    * `./build.sh`
3. Make install:
    * `sudo make install`

## Current development status ##

On the continuum of...

1. Compiles
2. Runs
3. Behaves correctly
4. Behaves correctly and runs fast

...it's somewhere between 2 and 3 right now. Although it's much faster than ack in my benchmarks.

    ack -i SOLR ~/cloudkick/reach  2.89s user 0.77s system 97% cpu 3.765 total

    ag -i SOLR ~/cloudkick/reach  0.25s user 0.13s system 94% cpu 0.404 total

You can use this with [my fork](https://github.com/ggreer/AckMate) of the popular ackmate plugin, which lets you use both ack and ag for searching in Textmate. You can also move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`


### TODO ###
* behave better when matching in files with really long lines
  * maybe say "match found at position X of line N" if line is > 10k chars
* allow searching files > 1GB in size
* optimizations
  * pthreads. these days it's silly to use only one core
* actually get textmate dir patterns working (this requires ruby regexes. not fun)

### Other stuff you might like ###
* [Ack](https://github.com/petdance/ack) - Better than grep
* [AckMate](https://github.com/protocool/AckMate) - An ack-powered replacement for TextMate's slow built-in search.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://book.git-scm.com/4_finding_with_git_grep.html) - As fast as Ag but only works on git repos.
