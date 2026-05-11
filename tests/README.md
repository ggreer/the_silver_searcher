The Silver Searcher test suite
==============================


Prerequisites
-------------

The test suite uses cram, a simple framework for testing command line applications.  
This framework uses a set of Python scripts. It requires Python 3.3 or newer.
The test definition files are text files with the .t extension.

### Installing cram

Some Unix distributions include a pre-built package. Example for Ubuntu:

    sudo apt install python3-cram

If not, the simplest way is to use pip:

    pip install cram

For more information, see the [cram home page on PyPI](https://pypi.org/project/cram/).

It's also possible to install cram from source:

    git clone https://github.com/aiiie/cram.git
    cd cram
    which python || sudo ln -s /usr/bin/python3 /usr/bin/python
    make
    sudo make install

For more information, see the [cram home page on GitHub](https://github.com/aiiie/cram/).


Testing the Unix version of ag
------------------------------

All ag test files invoke an ag instance at the base of the ag project tree.

For testing a version of ag that was just built in Unix, simply run at the base of the project tree:

    make test

For testing a pre-built version of ag installed in your PATH, you should store at the base of the ag project tree
an ag shell script containing something like this:

    #!/bin/bash
    /usr/bin/ag

Then manually run the tests like this:

    cram -v tests/*.t


Testing the Windows version of ag.exe
-------------------------------------

The cram test tool does not run in Windows. But it is possible to test ag.exe with cram within the WSL subsystem.
For that,
* Install cram in the WSL subsytem.
* Use an ag shell script that hides the main differences between the Windows and Unix versions of ag.
  Store this ag script at the base of the ag project tree, with contents like this:

    #!/bin/bash
    /MY/SRC/The_Silver_Searcher/bin/WIN64/ag.exe --unix "$@" | tr '\\' '/' | sed 's/C:\//\//g' ; test ${PIPESTATUS[0]} -eq 0

This example assumes that the C:\MY directory in Windows is also visible as /MY in WSL.
For that, create symbolic links with the same name (MY here) in the roots of the Windows and WSL file systems,
pointing to the same physical directory. (Ex: `C:\Users\YOURNAME\Documents` and `/mnt/c/Users/YOURNAME/Documents` resp.)

The --unix option is a hidden option added specifically for testing ag.exe within WSL. It changes the output on both
stdout and stderr to untranslated, to avoid outputing the \r characters that confuse cram.

Also for the ag.exe test to succeed, it's necessary to tell cram to run the tests in a temporary directory that is
visible from both sides of the WSL world. For example `C:\MY\Temp` <=> `/MY/Temp`.
This can be automated with a cram script like this:

    #!/bin/bash
    TMPDIR=/MY/Temp cram "$@"

With all the above scripts prepared, run the tests like this:

    ./cram -v tests/*.t

Even with these scripts, do expect a few tests to fail, as, for example, the error messages in Windows are not
worded exactly as in Unix. Likewise, the output colouring does not work the same way. Etc.
