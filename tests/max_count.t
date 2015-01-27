Setup:

  $ . $TESTDIR/setup.sh
  $ echo "blah" > blah.txt
  $ echo "blah2" >> blah.txt
  $ echo "blah2" > blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt
  $ echo "blah2" >> blah2.txt # 10 lines

Max match of 1:

  $ ag --max-count 1 blah blah.txt
  ERR: Too many matches in blah.txt. Skipping the rest of this file.
  1:blah

Max match of 10:

  $ ag --count --max-count 10 blah blah2.txt
  ERR: Too many matches in blah2.txt. Skipping the rest of this file.
  blah2.txt:10
