Setup:

  $ . $TESTDIR/setup.sh
  $ printf '%s\n' 'foo' 'foobar' 'foo bar' > ./option_wordregexp_test.txt

Word matching, literal:

  $ ag -Qw 'foo' ./option_wordregexp_test.txt
  1:foo
  3:foo bar
  $ ag -Qw 'bar' ./option_wordregexp_test.txt
  3:foo bar

Word matching, non-literal:

  $ ag -w 'f.+o' ./option_wordregexp_test.txt
  1:foo
  3:foo bar
  $ ag -w 'b.+r' ./option_wordregexp_test.txt
  3:foo bar


