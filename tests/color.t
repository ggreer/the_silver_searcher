Setup:

  $ . $TESTDIR/setup.sh
  $ echo foo > ./blah.txt
  $ echo bar >> ./blah.txt

Matches should contain colors:

  $ ag --no-numbers --color foo blah.txt
  \x1b[30;43mfoo\x1b[0m\x1b[K (esc)

--nocolor should suppress colors:

  $ ag --nocolor foo blah.txt
  1:foo

--invert-match should suppress colors:

  $ ag --invert-match foo blah.txt
  2:bar
