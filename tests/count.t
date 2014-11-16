Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"
  $ echo "blah" > blah.txt
  $ echo "blah2" >> blah.txt
  $ echo "blah_OTHER" > other_file.txt
  $ echo "blah_OTHER" >> other_file.txt

Count matches:

  $ ag --count --parallel blah | sort
  blah.txt:2
  other_file.txt:2

Count stream matches:

  $ echo 'blah blah blah' | ag --count blah
  3

Count stream matches per line (not very useful since it does not print zero):

  $ cat blah.txt | ag --count blah
  1
  1
