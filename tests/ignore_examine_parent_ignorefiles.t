Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p subdir
  $ printf 'match1\n' > subdir/file1.txt
  $ printf 'file1.txt\n' > .ignore
  $ mkdir -p outer
  $ printf 'x2\n' > outer/file2.txt
  $ printf 'file2.txt\n' > subdir/.ignore
  $ mkdir -p outer/subouter
  $ printf 'y3\n' > outer/subouter/file3.txt
  $ printf 'y4\n' > outer/subouter/file4.txt
  $ printf 'file3.txt\n' > outer/.ignore

Ignore patterns are retained when descending into subdirectories:

  $ ag match
  [1]

Current directory ignores get examined when searching only a subdirectory (#144):

  $ ag match subdir
  [1]

Parent directory ignores get examined (#144):

  $ cd subdir
  $ ag match
  [1]

Current directory ignores are not applied to an outer directory:

  $ ag x ../outer
  ../outer/file2.txt:1:x2

For an outer directory, its parent directory ignores get examined:

  $ ag y ../outer/subouter
  ../outer/subouter/file4.txt:1:y4
