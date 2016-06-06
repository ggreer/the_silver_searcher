Setup:

  $ . $TESTDIR/../setup.sh
  $ cp $TESTDIR/test_recursive.zip .

Searching zip files is not standard

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel TEST
  [1]

  $ $TESTDIR/../../ag -z --nocolor --workers=1 --parallel TEST
  test_recursive.zip/a/a.zip/a/b.txt:1:TEST
  test_recursive.zip/a.txt:1:TEST
