Setup:

  $ . $TESTDIR/setup.sh
  $ printf '%s\n' 'foo' 'foobar' 'foo bar' > ./option_lineregexp_test.txt

Line matching, literal:

  $ ag -Qx 'foo' ./option_lineregexp_test.txt
  1:foo
  $ ag -Qx 'bar' ./option_lineregexp_test.txt
  [1]

Line matching, non-literal:

  $ ag -x 'f.+o' ./option_lineregexp_test.txt
  1:foo
  $ ag -x 'b.+r' ./option_lineregexp_test.txt
  [1]


