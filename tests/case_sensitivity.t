Setup:

  $ . $TESTDIR/setup.sh
  $ echo Foo >> ./sample
  $ echo bar >> ./sample

Case sensitive by default:

  $ ag foo sample
  [1]
  $ ag FOO sample
  [1]
  $ ag 'f.o' sample
  [1]
  $ ag Foo sample
  1:Foo
  $ ag 'F.o' sample
  1:Foo

Case insensitive mode:

  $ ag foo -i sample
  1:Foo
  $ ag foo --ignore-case sample
  1:Foo
  $ ag 'f.o' -i sample
  1:Foo
