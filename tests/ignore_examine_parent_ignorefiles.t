Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p subdir
  $ echo 'match1' > subdir/file1.txt
  $ echo 'file1.txt' > .gitignore

Ignore directory specified by name:

  $ ag match
  [1]

# Ignore directory specified by name in parent directory when using path (#144):

#   $ ag match subdir
#   [1]

# Ignore directory specified by name in parent directory when using current directory (#144):

#   $ cd subdir
#   $ ag match
#   [1]
