Setup:

  $ source $TESTDIR/setup.sh
  $ echo 'valid: 1' > ./blah.txt
  $ echo 'some_string' >> ./blah.txt
  $ echo 'valid: 654' >> ./blah.txt
  $ echo 'some_other_string' >> ./blah.txt
  $ echo 'valid: 0' >> ./blah.txt
  $ echo 'valid: 23' >> ./blah.txt
  $ echo 'valid: 0' >> ./blah.txt

Search for lines not matching "valid: 0" in blah.txt:

  $ ag -v 'valid: 0' ./blah.txt
  blah.txt:1:whatever1
