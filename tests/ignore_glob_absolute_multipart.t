Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p parent/multi-part
  $ mkdir -p parent2/more-multi/part
  $ echo 'match1' > parent/multi-part/file1.txt
  $ echo 'match2' > parent2/more-multi/part/file2.txt
  $ echo 'parent/multi-*\nparent*/more-multi/part' > .gitignore

Ignore directory specified by glob:

  $ ag match .
  [1]

Ignore directory specified by glob with absolute search path (#448):

  $ ag match $(pwd)
  [1]
