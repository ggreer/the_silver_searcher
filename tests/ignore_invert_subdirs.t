Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b ./a/c
  $ printf '/a/*\n' >> ./.gitignore
  $ printf '!a/printme.txt\n' >> ./.gitignore
  $ printf 'blah1\n' > ./a/printme.txt
  $ printf 'blah2\n' > ./a/dontprintme.txt

Ignore .gitignore patterns but not .ignore patterns:

  $ ag blah
  a/printme.txt:1:blah1
