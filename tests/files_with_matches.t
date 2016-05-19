Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\n' > ./foo.txt
  $ printf 'bar\n' > ./bar.txt

Files with matches:

  $ ag --files-with-matches foo foo.txt
  foo.txt
  $ ag --files-with-matches foo foo.txt bar.txt
  foo.txt
  $ ag --files-with-matches foo bar.txt
  [1]

Files without matches:

  $ ag --files-without-matches bar foo.txt
  foo.txt
  $ ag --files-without-matches bar foo.txt bar.txt
  foo.txt
  $ ag --files-without-matches bar bar.txt
  [1]
