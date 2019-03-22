Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\n' > ./foo.txt
  $ printf 'bar\n' > ./bar.txt
  $ printf 'foo\nbar\nbaz\n' > ./baz.txt
  $ printf 'duck\nanother duck\nyet another duck\n' > ./duck.txt
  $ cp duck.txt goose.txt
  $ echo "GOOSE!!!" >> ./goose.txt

Files with matches:

  $ ag --files-with-matches foo foo.txt
  foo.txt
  $ ag --files-with-matches foo foo.txt bar.txt
  foo.txt
  $ ag --files-with-matches foo bar.txt
  [1]
  $ ag --files-with-matches foo foo.txt bar.txt baz.txt
  foo.txt
  baz.txt
  $ ag --files-with-matches bar foo.txt bar.txt baz.txt
  bar.txt
  baz.txt
  $ ag --files-with-matches foo bar.txt baz.txt
  baz.txt

Files without matches:
(Prints names of files in which no line matches query)

  $ ag --files-without-matches bar foo.txt
  foo.txt
  $ ag --files-without-matches bar foo.txt bar.txt
  foo.txt
  $ ag --files-without-matches bar bar.txt
  [1]
  $ ag --files-without-matches foo foo.txt bar.txt baz.txt
  bar.txt
  $ ag --files-without-matches bar foo.txt bar.txt baz.txt
  foo.txt

Files with inverted matches:
(Prints names of files in which some line doesn't match query)

  $ ag --files-with-matches --invert-match bar bar.txt
  [1]
  $ ag --files-with-matches --invert-match foo foo.txt bar.txt baz.txt
  bar.txt
  baz.txt
  $ ag --files-with-matches --invert-match bar foo.txt bar.txt baz.txt
  foo.txt
  baz.txt

Files without inverted matches:
(Prints names of files in which no line doesn't match query,
 i.e. where every line matches query)

  $ ag --files-without-matches --invert-match duck duck.txt
  duck.txt
  $ ag --files-without-matches --invert-match duck goose.txt
  [1]
  $ ag --files-without-matches --invert-match duck duck.txt goose.txt
  duck.txt
