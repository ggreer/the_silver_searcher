Setup:

  $ . $TESTDIR/setup.sh
  $ echo "the quick brown foxy" > blah.txt
  $ echo "another foxlike word" >> blah.txt
  $ echo "no matches here" >> blah.txt

Only print matches:

  $ ag -o "fox\w+"
  blah.txt:1:foxy
  blah.txt:2:foxlike
