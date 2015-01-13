Setup.
  $ . $TESTDIR/setup.sh
  $ touch empty.txt
  $ echo foo > nonempty.txt

Zero-length match on an empty file should fail silently with return code 1

  $ ag "^" empty.txt
  [1]

A genuine zero-length match should succeed:
  $ ag "^" nonempty.txt
  1:foo
