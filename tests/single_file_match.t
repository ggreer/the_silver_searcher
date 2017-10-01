Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\n' > ./foo.txt

Without filename argument:

  $ ag foo foo.txt
  1:foo

With filename argument:

  $ ag --filename foo foo.txt
  foo.txt
  1:foo
