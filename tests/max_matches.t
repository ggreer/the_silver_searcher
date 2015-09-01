Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'test' > ./blah.txt
  $ echo 'test' >> ./blah.txt
  $ echo 'lala' > ./blah2.txt
  $ echo 'test' >> ./blah2.txt
  $ echo 'lala' >> ./blah2.txt

Limit to 1 match, one file:

  $ ag -j 1 test blah.txt
  1:test

Limit to 1 match, multiple files:

  $ ag -j 1 test blah.txt blah2.txt
  blah.txt:1:test

Limit matches to a number greater than total possible matches:

  $ ag -j 100 test blah.txt
  1:test
  2:test

Limit matches using long option:

  $ ag --max-matches 1 test blah.txt
  1:test
