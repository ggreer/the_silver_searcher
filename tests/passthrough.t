Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"
  $ echo "foo bar" > passthrough_test.txt
  $ echo "zoo zar" >> passthrough_test.txt
  $ echo "foo test" >> passthrough_test.txt

No impact on non-stream:

  $ ag --passthrough zoo passthrough_test.txt
  zoo zar

Match stream with --passthrough:

  $ cat passthrough_test.txt | ag --passthrough foo
  foo bar
  zoo zar
  foo test

Match stream without --passthrough:

  $ cat passthrough_test.txt | ag foo
  foo bar
  foo test
