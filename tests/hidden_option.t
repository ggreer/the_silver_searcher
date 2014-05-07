Setup:

  $ source $TESTDIR/setup.sh
  $ mkdir hidden_bug
  $ cd hidden_bug
  $ echo "test" > a.txt
  $ git init --quiet
  $ if [ ! -d .git/info ] ; then mkdir .git/info ; fi
  $ echo "a.txt" > .git/info/exclude

Do not ignore a.txt

  $ ag --hidden --ignore-dir .git test
  a.txt:1:test

