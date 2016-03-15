Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'valid: 1\n' > ./blah.txt
  $ printf 'some_string\n' >> ./blah.txt
  $ printf 'valid: 654\n' >> ./blah.txt
  $ printf 'some_other_string\n' >> ./blah.txt
  $ printf 'valid: 0\n' >> ./blah.txt
  $ printf 'valid: 23\n' >> ./blah.txt
  $ printf 'valid: 0\n' >> ./blah.txt

Search for lines not matching "valid: 0" in blah.txt:

  $ ag -v 'valid: '
  blah.txt:2:some_string
  blah.txt:4:some_other_string
