Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"
  $ echo "foo bar" > stdin_test.txt
  $ echo "zoo zar" >> stdin_test.txt
  $ echo "foo test" >> stdin_test.txt

Ignore stdin if file is given:

  $ echo foobar | ag foo stdin_test.txt
  1:foo bar
  3:foo test
