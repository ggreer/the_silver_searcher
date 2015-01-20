Setup:

  $ . $TESTDIR/setup.sh
  $ echo "the quick brown foxy" > blah.txt
  $ echo "blah blah blah" >> blah.txt
  $ echo "another foxlike word" >> blah.txt
  $ echo "no matches here" >> blah.txt
  $ echo "blah blah blah" >> blah.txt

Only print matches:

  $ ag -o "fox\w+" blah.txt
  foxy
  foxlike

Ditto but with filename & line numbers

  $ ag -o --noheading "fox\w+"
  blah.txt:1:foxy
  blah.txt:3:foxlike

Ditto but with filename at top

  $ ag -o -H "fox\w+"
  blah.txt
  1:foxy
  3:foxlike

Always print matches on separate lines:

  $ ag -o "blah" blah.txt
  blah
  blah
  blah
  blah
  blah
  blah

Ditto but with filename & line numbers

  $ ag -o --noheading "blah"
  blah.txt:2:blah
  blah.txt:2:blah
  blah.txt:2:blah
  blah.txt:5:blah
  blah.txt:5:blah
  blah.txt:5:blah

Ditto but with filename at top

  $ ag -o -H "blah"
  blah.txt
  2:blah
  2:blah
  2:blah
  5:blah
  5:blah
  5:blah

With columns

  $ ag -o -H --column "blah"
  blah.txt
  2:1:blah
  2:6:blah
  2:11:blah
  5:1:blah
  5:6:blah
  5:11:blah
