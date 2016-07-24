Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'blah\n' > ./blah.txt

Pipe + file = ignore pipe:

  $ printf 'bleh' | ag 'bl' ./blah.txt
  1:blah
