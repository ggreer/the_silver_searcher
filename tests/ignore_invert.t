Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'blah1\n' > ./printme.txt
  $ printf 'blah2\n' > ./dontprintme.c
  $ printf '*\n' > ./.wildcard-ignore
  $ printf '!*.txt\n' >> ./.wildcard-ignore
  $ printf '/*\n' > ./.rooted-ignore
  $ printf '!*.txt\n' >> ./.rooted-ignore

Inverted ignore with wildcard:

  $ ag -p .wildcard-ignore blah
  printme.txt:1:blah1

Inverted ignore with rooted wildcard:

  $ ag -p .rooted-ignore blah
  printme.txt:1:blah1
