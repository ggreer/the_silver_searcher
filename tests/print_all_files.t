Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\n' > ./foo.txt
  $ printf 'bar\n' > ./bar.txt
  $ printf 'baz\n' > ./baz.txt

All files:

  $ ag --print-all-files --group foo | sort
  
  
  1:foo
  bar.txt
  baz.txt
  foo.txt
