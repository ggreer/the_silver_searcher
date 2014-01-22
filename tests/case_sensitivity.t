Setup:

  $ source $TESTDIR/setup.sh
  $ echo Foo >> ./sample
  $ echo bar >> ./sample

Case sensitive by default:

  $ ag foo sample
  $ ag FOO sample
  $ ag 'f.o' sample
  $ ag Foo sample
  Foo
  $ ag 'F.o' sample
  Foo

Case insensitive mode:

  $ ag foo -i sample
  Foo
  $ ag foo --ignore-case sample
  Foo
  $ ag 'f.o' -i sample
  Foo
