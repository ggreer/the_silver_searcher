Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p parent/multi-part
  $ printf 'match1\n' > parent/multi-part/file1.txt
  $ printf 'parent/multi-*\n' > .gitignore

# Ignore directory specified by glob:

#   $ ag match .
#   [1]

# Ignore directory specified by glob with absolute search path (#448):

#   $ ag match $(pwd)
#   [1]
