# The Silver Searcher for Windows

## Specificities of this fork

This repository is a fork of [Geoff Greer](https://github.com/ggreer)'s [The Silver Searcher](https://github.com/ggreer/the_silver_searcher) for Unix, also known as 'ag'.

It is dedicated to building a well behaved version of ag.exe for Windows.  
The original version can be built for Windows using MinGW, but it has several serious shortcomings that limit its usefulness for Windows users world-wide.  
This version has the following improvements:

* Use and display Windows paths with the \ character.
* Expand pathnames with wildcards. (As, contrary to Unix shells, cmd.exe and PowerShell don't expand them.) 
* The -? option displays help.
* The -V/--version option displays details about this port, and every library used.
* Support command line arguments with non-ASCII characters in any console code page. 
* Correctly display pathnames and matching strings with non-ASCII characters in any console code page, even if they're not part of the code page.
* Use PCRE UTF8 option, allowing to search for non-ASCII regular expressions. (Like "." matching 1 character = 1 to 4 bytes!)
* Support Unicode escape sequences \uXXXX and \UXXXXXXXX in the search pattern.
* Support the three text files encodings standard in each version of Windows, the system code page, UTF-8, and UTF-16, and display matches correctly for all.
  (Dynamically detects each file encoding.) (The system code page is Windows-localization-specific. Ex: Code page 1252 for the USA version.)  
  Note that searching in UTF-16 files has a performance cost, as the text is converted to UTF-8 first. This is not the case for the other supported encodings.
* Also support UTF-8 and the current console code page encodings for text piped through the standard input.
  (The console code page being often different from the system code page. Ex: Code page 437 by default for the USA version.)
* Support pathnames longer than 260 characters, up to 64 KB on NTFS volumes, even on old versions of Windows like XP and 7.
* Support Windows junctions and symbolic links.
* Support 64-bit statistics even for the 32-bit build. (Allows searching through more than 4GB of files with the x86 build.)
* The debug version displays the thread number ahead of each debug message.
* Can be built with Microsoft Visual Studio versions 2013 to 2019, without any dependency on outside libraries.
* Last but not least, everything (ag.exe with all its provided dependent libraries) is built with a single make.bat command.

Versions of Windows supported:

* The 32-bits version runs in Windows XP, and all later versions of Windows.  
  The 64-bits version runs in all AMD64 versions of Windows.

Thanks to [Krzysztof Kowalczyk](https://github.com/kjk) who did the original [Native Visual Studio Port](https://github.com/kjk/the_silver_searcher).
I've actually started from his August 2016 code, and kept improving it.

For instructions on how to build this code, and with what tools, see [win32/README.md](win32/README.md).

Jean-François Larvoire


## Original introduction

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

* openSUSE

        zypper install the_silver_searcher

* CentOS

        yum install the_silver_searcher

* NixOS/Nix/Nixpkgs

        nix-env -iA silver-searcher

* SUSE Linux Enterprise: Follow [these simple instructions](https://software.opensuse.org/download.html?project=utilities&package=the_silver_searcher).


### BSD

* FreeBSD

        pkg install the_silver_searcher

* OpenBSD/NetBSD

        pkg_add the_silver_searcher

### Windows

* Win32/64

  Unofficial daily builds are [available](https://github.com/k-takata/the_silver_searcher-win32).
  
  Another version with additional features optimized for Windows is available [there](https://github.com/JFLarvoire/the_silver_searcher/releases).

  * Supports text files encoded in both Windows System code page (Ex: CP 1252) and UTF-8.
  * Supports unicode search strings, and outputs Unicode results.
  * Supports unicode pathname > 260 characters, junctions, and symbolic links.
  
* winget

        winget install "The Silver Searcher"
  
  Notes:
  - This installs a [release](https://github.com/JFLarvoire/the_silver_searcher/releases) of ag.exe optimized for Windows.
  - winget.exe now is the default command-line package manager for Windows 10.  
    It is supported on Windows 10 build 1709 or newer.  
    If needed, it can be installed from the Microsoft Store, where it is called "App Installer".  
    It can also be installed manually using instructions on its [github home](https://github.com/microsoft/winget-cli).
  - The setup script in the Ag's winget package installs ag.exe in the first directory that matches one of these criteria:
     1. Over a previous instance of ag.exe *from the same [origin](https://github.com/JFLarvoire/the_silver_searcher)* found in the PATH
     2. In the directory defined in environment variable bindir_%PROCESSOR_ARCHITECTURE%
     3. In the directory defined in environment variable bindir
     4. In the directory defined in environment variable windir
  
* Chocolatey

        choco install ag

  Notes:
  - This installs the [default build](https://github.com/ggreer/the_silver_searcher/releases) of ag.exe,
    without the bug fixes present in the [optimized build](https://github.com/JFLarvoire/the_silver_searcher/releases).

* MSYS2

        pacman -S mingw-w64-{i686,x86_64}-ag

* Cygwin

  Run the relevant [`setup-*.exe`](https://cygwin.com/install.html), and select "the\_silver\_searcher" in the "Utils" category.

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
            yum -y install pcre-devel xz-devel zlib-devel

    * openSUSE:

            zypper source-install --build-deps-only the_silver_searcher  

    * Windows with Visual C++:

            Nothing to do. All necessary libraries are provided in the [win32](win32/README.md) subdirectory.

    * Windows with MinGW:  

            It's complicated. See [this wiki page](https://github.com/ggreer/the_silver_searcher/wiki/Windows).

2. Run the build script (which just runs aclocal, automake, etc):
    * In all Unix versions:

        ./build.sh

    * On Windows (inside a cmd shell with Microsoft Visual C++, using [this](https://github.com/JFLarvoire/the_silver_searcher) set of sources):

        win32\make.bat

    * On Windows (inside an msys/MinGW shell with gcc):

        make -f Makefile.w32  

3. Make install:
    * In all Unix versions, and Windows with MinGW:

        sudo make install

    * On Windows (inside a cmd shell with Microsoft Visual C++):

        The 32-bit and 64-bit version are in bin\WIN32\ag.exe and bin\WIN64\ag.exe respectively.


### Building a release tarball

GPG-signed releases are available [here](http://geoff.greer.fm/ag).

Building release tarballs requires the same dependencies, except for automake and pkg-config. Once you've installed the dependencies, just run:

    ./configure
    make
    make install

You may need to use `sudo` or run as root for the make install.


## Editor Integration

### Vim

You can use Ag with [ack.vim](https://github.com/mileszs/ack.vim) by adding the following line to your `.vimrc`:

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

* [Ack](https://github.com/petdance/ack3) - Better than grep. Without Ack, Ag would not exist.
* [ack.vim](https://github.com/mileszs/ack.vim)
* [Exuberant Ctags](http://ctags.sourceforge.net/) - Faster than Ag, but it builds an index beforehand. Good for *really* big codebases.
* [Git-grep](http://git-scm.com/docs/git-grep) - As fast as Ag but only works on git repos.
* [fzf](https://github.com/junegunn/fzf) - A command-line fuzzy finder 
* [ripgrep](https://github.com/BurntSushi/ripgrep)
* [Sack](https://github.com/sampson-chen/sack) - A utility that wraps Ack and Ag. It removes a lot of repetition from searching and opening matching files.
