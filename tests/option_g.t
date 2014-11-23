Setup:

  $ . $TESTDIR/setup.sh
  $ touch foobar

Search for lines matching "hello" in test_vimgrep.txt:

  $ ag -g foobar
  foobar
  $ ag -g baz
  [1]
