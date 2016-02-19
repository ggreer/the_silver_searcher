Setup:

  $ . $TESTDIR/setup.sh
  $ cp $TESTDIR/test_file.zip .

Searching zip files is not standard

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel test
  [1]

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel -z test
  test_file.zip/test.txt:1:test
