Setup.
  $ . $TESTDIR/setup.sh
  $ touch empty.txt
  $ printf 'foo\n' > nonempty.txt

Zero-length match on an empty file should fail silently with return code 1

  $ ag "^" empty.txt
  [1]

A genuine zero-length match should succeed:
  $ ag "^" nonempty.txt
  1:foo

Empty files should be listed with --unrestricted --files-with-matches (-ul)
  $ ag -lu --stats | sed '$d' | sort # Remove the last line about timing which will differ
  2 files contained matches
  2 files searched
  2 matches
  4 bytes searched
  empty.txt
  nonempty.txt
