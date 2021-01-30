Setup. Note that we have to turn --color on manually since ag detects that
stdout isn't a tty when running in cram.

  $ . $TESTDIR/setup.sh
  $ alias ag="$TESTDIR/../ag --noagrc --noaffinity --workers=1 --parallel --color"
  $ printf 'foo\n' > ./blah.txt
  $ printf 'bar\n' >> ./blah.txt

Matches should contain colors:

  $ ag --no-numbers foo blah.txt
  \x1b[30;43mfoo\x1b[0m\x1b[K (esc)

--nocolor should suppress colors:

  $ ag --nocolor foo blah.txt
  1:foo

--invert-match should suppress colors:

  $ ag --invert-match foo blah.txt
  2:bar

-v is the same as --invert-match

  $ ag -v foo blah.txt
  2:bar
