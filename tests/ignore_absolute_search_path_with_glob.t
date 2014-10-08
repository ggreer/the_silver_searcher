Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p parent/multi-part
  $ echo 'match1' > parent/multi-part/file1.txt
  $ echo 'parent/multi-*' > .gitignore

Ignore directory specified by glob:

  $ ag match .
  [1]

Ignore directory specified by glob with absolute search path (#448):

  $ ag match $(pwd)
  [1]
