Setup:

  $ . $TESTDIR/setup.sh
  $ echo "blah\nblah2" > blah.txt

Ensure column is correct:

  $ ag --column "blah\nb"
  blah.txt:1:1:blah
  blah.txt:2:0:blah2

# Test ackmate output

#   $ ag --ackmate "blah\nb"
#   :blah.txt
#   1;1 4:blah
#   2;1 2:blah2
