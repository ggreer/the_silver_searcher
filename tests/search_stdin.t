Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'blah' > ./blah.txt

Feed blah.txt from stdin:

  $ ag 'blah' < ./blah.txt
  blah.txt:1:blah
