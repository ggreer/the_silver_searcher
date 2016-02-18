Setup:

  $ . $TESTDIR/setup.sh
  $ cp $TESTDIR/test_folder.zip .

Searching zip files is not standard

  $ ag a
  [1]

  $ ag -z a
  test_folder.zip/sub/a.txt:1:a
