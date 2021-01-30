Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --noagrc --noaffinity --nocolor --workers=1"
  $ printf "blah\n" > blah.txt
  $ printf "blah2\n" >> blah.txt
  $ printf "blah_OTHER\n" > other_file.txt
  $ printf "blah_OTHER\n" >> other_file.txt

Count matches:

  $ ag --count --parallel blah | sort
  blah.txt:2
  other_file.txt:2

Count stream matches:

  $ printf 'blah blah blah\n' | ag --count blah
  3

Count stream matches per line (not very useful since it does not print zero):

  $ cat blah.txt | ag --count blah
  1
  1
