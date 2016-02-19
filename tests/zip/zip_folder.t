Setup:

  $ . $TESTDIR/../setup.sh
  $ cp $TESTDIR/test_folder.zip .

Searching zip files is not standard

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel a
  [1]

  $ $TESTDIR/../../ag -z --nocolor --workers=1 --parallel a
  test_folder.zip/sub/a.txt:1:a
