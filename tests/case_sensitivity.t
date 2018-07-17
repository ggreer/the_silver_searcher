Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'Foo\n' >> ./sample
  $ printf 'bar\n' >> ./sample
  $ printf '\S\n' >> ./sample
  $ printf '\s\n' >> ./sample

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
  $ ag 'fo\S' sample
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

  $ ag fOO -i sample
  1:Foo
  $ ag fOO --ignore-case sample
  1:Foo
  $ ag 'f.o' -i sample
  1:Foo

Case insensitive file regex

  $ ag -i  -g 'Samp.*'
  sample

Smart case literal mode
  $ ag -Q "\S" sample
  3:\S
  $ ag -Q "\s" sample
  3:\S
  4:\s
