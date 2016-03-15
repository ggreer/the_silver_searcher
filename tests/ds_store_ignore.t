Setup.
  $ . $TESTDIR/setup.sh
  $ mkdir -p dir0/dir1/dir2
  $ printf '*.DS_Store\n' > dir0/.gitignore
  $ printf 'blah\n' > dir0/dir1/dir2/blah.txt
  $ touch dir0/dir1/.DS_Store

Find blah in blah.txt

  $ ag blah
  dir0/dir1/dir2/blah.txt:1:blah
