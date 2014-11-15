Setup:

  $ echo "blah" > blah.txt
  $ echo "blah2" >> blah.txt
  $ echo "blah_OTHER" > other_file.txt
  $ echo "blah_OTHER" >> other_file.txt

Count matches:

  $ $TESTDIR/../ag --nocolor --workers=1 --count --parallel blah
  blah.txt:2
  other_file.txt:2

Count stream matches:

  $ echo 'blah blah blah' | $TESTDIR/../ag --nocolor --workers=1 --count blah
  3

Count stream matches per line (not very useful since it does not print zero):

  $ cat blah.txt | $TESTDIR/../ag --nocolor --workers=1 --count blah
  1
  1
