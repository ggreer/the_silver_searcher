Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\n' > ./foo.txt
  $ printf 'bar\n' > ./bar.txt
  $ printf 'baz\n' > ./baz.txt

All files:

  $ ag --all-files --group foo
  bar.txt
  
  baz.txt
  
  foo.txt
  1:foo
