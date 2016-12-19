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
The sed is used to remove the timing line from --stats which will vary
Sort the files list since readdir() returns in an unknown order
Note that output.txt is created by the shell before starting ag, so it is
part of the expected output
  $ ag -lu --stats | sed '$d' > output.txt
  $ grep -v '^[0-9]' output.txt | sort
  empty.txt
  nonempty.txt
  output.txt
  $ grep '^[0-9]' output.txt
  3 matches
  3 files contained matches
  3 files searched
  4 bytes searched
