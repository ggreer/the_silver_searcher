Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'blah1\n' > ./printme.txt
  $ printf 'blah2\n' > ./dontprintme.c
  $ printf '*\n' > ./.ignore
  $ printf '!*.txt\n' >> ./.ignore

Ignore .gitignore patterns but not .ignore patterns:

  $ ag blah
  printme.txt:1:blah1
