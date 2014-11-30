Setup:

  $ . $TESTDIR/setup.sh
  $ echo "blah" > blah.txt
  $ echo "blah2" >> blah.txt

Max match of 1:

  $ ag --max-count 1 blah
  ERR: Too many matches in ./blah.txt. Skipping the rest of this file.
  blah.txt:1:blah
