Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'foo' > ./blah.txt
  $ echo       >> ./blah.txt

This file structure caused older versions of ag to falsely report an
inverted match for "foo"

  $ ag -L 'foo' ./blah.txt
  [1]

Run a few more tests....

  $ echo 'stuff for a test'    >  ./blah2.txt
  $ echo 'There you go'        >> ./blah2.txt
  $ echo 'Down Goes Frazier'   >> ./blah2.txt
  $ echo 'There it is'         >  ./blah3.txt
  $ echo '.........'           >> ./blah3.txt
  $ echo 'Was it there'        >> ./blah3.txt

  $ ag -L 'there' ./blah.txt ./blah2.txt ./blah3.txt
  blah.txt

If -v and -L are both specified, -L wins ...

  $ ag -v -L 'stuff' ./blah.txt ./blah2.txt ./blah3.txt
  blah.txt
  blah3.txt
