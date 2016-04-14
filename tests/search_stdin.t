Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'blah\n' > ./blah.txt

Feed blah.txt from stdin:

  $ ag 'blah' < ./blah.txt
  blah.txt:1:blah
