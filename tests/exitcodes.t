Setup:

  $ . $TESTDIR/setup.sh
  $ echo foo > ./exitcodes_test.txt
  $ echo bar >> ./exitcodes_test.txt

Normal matching:

  $ ag foo exitcodes_test.txt
  1:foo
  $ ag zoo exitcodes_test.txt
  [1]

Inverted matching:

  $ ag -v foo exitcodes_test.txt
  2:bar
  $ ag -v zoo exitcodes_test.txt
  1:foo
  2:bar
  $ ag -v "foo|bar" exitcodes_test.txt
  [1]
