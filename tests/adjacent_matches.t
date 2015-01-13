Setup:

  $ . $TESTDIR/setup.sh
  $ alias ag="$TESTDIR/../ag --workers=1 --parallel --color"
  $ echo blahfoofooblah > ./fooblah.txt

Highlights are adjacent:

  $ ag --no-numbers foo
  \x1b[1;32mfooblah.txt\x1b[0m\x1b[K:blah\x1b[30;43mfoo\x1b[0m\x1b[K\x1b[30;43mfoo\x1b[0m\x1b[Kblah (esc)
