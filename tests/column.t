Setup:

  $ . $TESTDIR/setup.sh
  $ printf "blah\nblah2\n" > blah.txt

Ensure column is correct:

  $ ag --column "blah\nb"
  blah.txt:1:1:blah
  blah.txt:2:0:blah2

# Test ackmate output. Not quite right, but at least offsets are in the
# ballpark instead of being 9 quintillion

  $ ag --ackmate "lah\nb"
  :blah.txt
  1;blah
  2;1 5:blah2
