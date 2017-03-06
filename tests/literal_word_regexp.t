Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'blah abc def' > blah1.txt
  $ echo 'abc blah def' > blah2.txt
  $ echo 'abc def blah' > blah3.txt
  $ echo 'abcblah def' > blah4.txt
  $ echo 'abc blahdef' >> blah4.txt
  $ echo 'blahx blah' > blah5.txt
  $ echo 'abcblah blah blah' > blah6.txt

Match a word of the beginning:

  $ ag -wF --column 'blah' blah1.txt
  1:1:blah abc def

Match a middle word:

  $ ag -wF --column 'blah' blah2.txt
  1:5:abc blah def

Match a last word:

  $ ag -wF --column 'blah' blah3.txt
  1:9:abc def blah

No match:

  $ ag -wF --column 'blah' blah4.txt
  [1]

Match:

  $ ag -wF --column 'blah' blah5.txt
  1:7:blahx blah

Case of a word repeating the same part:

  $ ag -wF --column 'blah blah' blah6.txt
  1:9:abcblah blah blah
