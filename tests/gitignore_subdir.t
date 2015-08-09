Setup:

  $ . $TESTDIR/setup.sh
  $ git init --quiet
  $ echo "a.txt" > .gitignore
  $ echo "test" > a.txt
  $ mkdir sub1
  $ cd sub1

Make sure we correctly only ignore stuff in one folder

  $ echo '!a.txt' > .gitignore
  $ echo "testa" > a.txt
  $ cd ..

The output of git status tells us which files should be searched.
I add it so i can also check that gitignores behaviour did not change.
  $ git status --short --untracked-files=all
  ?? .gitignore
  ?? sub1/.gitignore
  ?? sub1/a.txt

The test
  $ ag test
  sub1/a.txt:1:testa

