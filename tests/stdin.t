Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"
  $ echo "foo bar" > stdin_test.txt
  $ echo "zoo zar" >> stdin_test.txt
  $ echo "foo test" >> stdin_test.txt
  $ echo "foobar" > stdin_test2.txt

Ignore stdin if file is given:

  $ echo foobar | ag foo stdin_test.txt
  1:foo bar
  3:foo test

  $ ag foo stdin_test.txt <stdin_test2.txt
  1:foo bar
  3:foo test

Input redirection from file:

  $ ag foo <stdin_test.txt
  (/dev/(stdin|fd/0):1:)?foo bar (re)
  (/dev/(stdin|fd/0):3:)?foo test (re)
