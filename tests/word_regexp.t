Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'foo' > blah.txt
  $ echo 'bar' >> blah.txt
  $ echo 'foobar' >> blah.txt

Word regexp:

  $ ag -w 'foo|bar' ./
  blah.txt:1:foo
  blah.txt:2:bar
