Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'file1\n' > .gitignore
  $ touch .git
  $ printf 'match1\n' > file1
  $ printf 'match2\n' > file2
  $ mkdir nested
  $ touch nested/.git
  $ printf 'file2\n'  > nested/.gitignore
  $ printf 'match1\n' > nested/file1
  $ printf 'match2\n' > nested/file2

Matching from parent repository uses parent .gitignore only for parent files and that of the nested one only for the files of the nested:

  $ ag match | sort
  file2:1:match2
  nested/file1:1:match1

Matching nested repository path does not use .gitignore of the parent repository:

  $ ag match nested
  nested/file1:1:match1

Matching from a nested repository does not use .gitignore of the parent repository:

  $ cd nested
  $ ag match
  file1:1:match1
