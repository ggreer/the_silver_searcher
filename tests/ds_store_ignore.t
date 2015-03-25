Setup.
  $ . $TESTDIR/setup.sh
  $ mkdir -p dir0/dir1/dir2
  $ echo '*.DS_Store' > dir0/.gitignore
  $ echo blah > dir0/dir1/dir2/blah.txt
  $ touch dir0/dir1/.DS_Store

Find blah in blah.txt

  $ ag blah
  dir0/dir1/dir2/blah.txt:1:blah
