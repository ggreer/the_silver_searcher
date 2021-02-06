Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --noaffinity --nocolor --workers=1"
  $ printf "foo bar\n" > passthrough_test.txt
  $ printf "zoo zar\n" >> passthrough_test.txt
  $ printf "foo test\n" >> passthrough_test.txt

No impact on non-stream:

  $ ag --passthrough zoo passthrough_test.txt
  2:zoo zar

Match stream with --passthrough:

  $ cat passthrough_test.txt | ag --passthrough foo
  foo bar
  zoo zar
  foo test

Match stream without --passthrough:

  $ cat passthrough_test.txt | ag foo
  foo bar
  foo test
