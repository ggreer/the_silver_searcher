Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir hidden_bug
  $ cd hidden_bug
  $ printf "test\n" > a.txt
  $ git init --quiet
  $ if [ ! -d .git/info ] ; then mkdir .git/info ; fi
  $ printf "a.txt\n" > .git/info/exclude

  $ ag --ignore-dir .git test
  [1]

  $ ag --hidden --ignore-dir .git test
  [1]

  $ ag -U --ignore-dir .git test
  a.txt:1:test

  $ ag --hidden -U --ignore-dir .git test
  a.txt:1:test

  $ mkdir -p ./.hidden
  $ printf 'whatever\n' > ./.hidden/a.txt

  $ ag whatever
  [1]

  $ ag --hidden whatever
  [1]

  $ printf "\n" > .git/info/exclude

  $ ag whatever
  [1]

  $ ag --hidden whatever
  .hidden/a.txt:1:whatever
