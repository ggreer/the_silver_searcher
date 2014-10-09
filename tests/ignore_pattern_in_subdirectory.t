Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir subdir
  $ echo 'first' > file1.txt
  $ echo 'second' > subdir/file2.txt
  $ echo '*.txt' > .gitignore

Ignore file based on extension match:

  $ ag first
  [1]

Ignore file in subdirectory based on extension match (#442):

  $ ag second
  [1]
