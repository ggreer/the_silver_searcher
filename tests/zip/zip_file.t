Setup:

  $ . $TESTDIR/setup.sh
  $ cp $TESTDIR/test_file.zip .

Searching zip files is not standard

  $ ag test
  [1]

  $ ag -z test
  test_file.zip/test.txt:1:test
