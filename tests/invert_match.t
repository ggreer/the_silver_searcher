Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'valid: 1' > ./blah.txt
  $ echo 'some_string' >> ./blah.txt
  $ echo 'valid: 654' >> ./blah.txt
  $ echo 'some_other_string' >> ./blah.txt
  $ echo 'valid: 0' >> ./blah.txt
  $ echo 'valid: 23' >> ./blah.txt
  $ echo 'valid: 0' >> ./blah.txt

Search for lines not matching "valid: 0" in blah.txt:

  $ ag -v 'valid: '
  blah.txt:2:some_string
  blah.txt:4:some_other_string
