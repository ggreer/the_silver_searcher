Setup:

  $ . $TESTDIR/setup.sh
  $ echo Foo >> ./sample
  $ echo bar >> ./sample

Smart case by default:

  $ ag foo sample
  1:Foo
  $ ag FOO sample
  [1]
  $ ag 'f.o' sample
  1:Foo
  $ ag Foo sample
  1:Foo
  $ ag 'F.o' sample
  1:Foo

Case sensitive mode:

  $ ag -s foo sample
  [1]
  $ ag -s FOO sample
  [1]
  $ ag -s 'f.o' sample
  [1]
  $ ag -s Foo sample
  1:Foo
  $ ag -s 'F.o' sample
  1:Foo
Case insensitive mode:

  $ ag foo -i sample
  1:Foo
  $ ag foo --ignore-case sample
  1:Foo
  $ ag 'f.o' -i sample
  1:Foo
