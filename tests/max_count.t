Setup:

  $ . $TESTDIR/setup.sh
  $ printf "blah\n" > blah.txt
  $ printf "blah2\n" >> blah.txt
  $ printf "blah2\n" > blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt
  $ printf "blah2\n" >> blah2.txt # 10 lines

Max match of 1:

  $ ag --max-count 1 blah blah.txt
  ERR: Too many matches in blah.txt. Skipping the rest of this file.
  1:blah

Max match of 10, one file:

  $ ag --count --max-count 10 blah blah2.txt
  ERR: Too many matches in blah2.txt. Skipping the rest of this file.
  10

Max match of 10, multiple files:

  $ ag --count --max-count 10 blah blah.txt blah2.txt
  ERR: Too many matches in blah2.txt. Skipping the rest of this file.
  blah.txt:2
  blah2.txt:10
