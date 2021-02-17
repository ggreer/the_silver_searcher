Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b ./a/c
  $ printf '/a\n' >> ./.gitignore
  $ printf '!a/dontprintme.txt\n' >> ./.gitignore
  $ printf 'blah1\n' > ./printme.txt
  $ printf 'blah2\n' > ./a/dontprintme.txt

Ignore .gitignore negation inside subdir if whole subdir is ignored:

  $ ag blah
  printme.txt:1:blah1
