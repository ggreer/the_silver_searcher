Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir subdir
  $ printf 'first\n' > file1.txt
  $ printf 'second\n' > subdir/file2.txt
  $ printf '*.txt\n' > .gitignore

Ignore file based on extension match:

  $ ag first
  [1]

Ignore file in subdirectory based on extension match (#442):

  $ ag second
  [1]
