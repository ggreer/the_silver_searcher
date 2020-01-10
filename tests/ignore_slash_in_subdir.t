Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p subdir/ignoredir
  $ mkdir ignoredir
  $ printf 'match1\n' > subdir/ignoredir/file1.txt
  $ printf 'match1\n' > ignoredir/file1.txt
  $ printf '/ignoredir\n' > subdir/.ignore

Ignore file in subdir/ignoredir, but not in ignoredir:

  $ ag match
  ignoredir/file1.txt:1:match1

From subdir, ignore file in subdir/ignoredir:

  $ cd subdir
  $ ag match
  [1]
