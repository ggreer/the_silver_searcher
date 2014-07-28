Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir hidden_bug
  $ cd hidden_bug
  $ echo "test" > a.txt
  $ git init --quiet
  $ if [ ! -d .git/info ] ; then mkdir .git/info ; fi
  $ echo "a.txt" > .git/info/exclude

  $ ag --ignore-dir .git test

  $ ag --hidden --ignore-dir .git test

  $ ag -U --ignore-dir .git test
  a.txt:1:test

  $ ag --hidden -U --ignore-dir .git test
  a.txt:1:test

  $ mkdir -p ./.hidden
  $ echo 'whatever' > ./.hidden/a.txt

  $ ag whatever

  $ ag --hidden whatever

  $ echo "" > .git/info/exclude

  $ ag whatever

  $ ag --hidden whatever
  .hidden/a.txt:1:whatever
