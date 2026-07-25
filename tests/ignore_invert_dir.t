Setup. This .gitignore idiom ignores extensionless files (posix binaries):
ignore everything, then un-ignore anything with a dot and all directories:

  $ . $TESTDIR/setup.sh
  $ printf '*\n' > ./.gitignore
  $ printf '!*.*\n' >> ./.gitignore
  $ printf '!*/\n' >> ./.gitignore
  $ printf 'match1\n' > ./printme.txt
  $ printf 'match2\n' > ./dontprintme
  $ mkdir ./subdir
  $ printf 'match3\n' > ./subdir/printme.c
  $ printf 'match4\n' > ./subdir/dontprintme

Un-ignore patterns for directories and files with extensions are honored:

  $ ag match
  printme.txt:1:match1
  subdir/printme.c:1:match3
