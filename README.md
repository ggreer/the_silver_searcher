# The Silver Searcher #

My attempt to make something better than ack, which itself is better than grep. Right now, it's not.

## Why use ag? ##

* It searches through code about 3x-5x faster than ack.
* It ignores files matched by patterns in your .gitignore and .hgignore.
* The binary name is 33% shorter than ack!
* It's written in C instead of perl, the lesser of two evils :)
* If there are files in your source repo that you don't want to search, just add their patterns to a .agignore file. \*cough\* extern \*cough\*

## Installation ##

1. Install PCRE development libraries:
    * Ubuntu: `apt-get install -y libpcre3-dev`
2. Run the build script:
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

    ag -i SOLR ~/cloudkick/reach  0.37s user 0.15s system 88% cpu 0.590 total

You can use this with [my fork](https://github.com/ggreer/AckMate) of the popular ackmate plugin, which lets you use both ack and ag for searching in textmate. You can also just move or delete `"~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"` and run `ln -s /usr/local/bin/ag "~/Library/Application Support/TextMate/PlugIns/AckMate.tmplugin/Contents/Resources/ackmate_ack"`


### TODO ###
* parse more ack options correctly
* actually get textmate dir patterns working
* support inverted matching
* support piping in stuff
* docs/manpages
* pthreads. these days it's silly to use only one core

### Other stuff you might like ###
* [Ack](https://github.com/petdance/ack)
* [AckMate](https://github.com/protocool/AckMate) - An ack-powered replacement for TextMate's slow built-in search.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://book.git-scm.com/4_finding_with_git_grep.html) - As fast as Ag but only works on git repos.
