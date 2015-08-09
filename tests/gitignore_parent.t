
Setup:

  $ . $TESTDIR/setup.sh
  $ git init --quiet
  $ echo "*.txt" > .gitignore
  $ mkdir root
  $ cd root
  $ echo "test" > a.txt
Make sure we ignore everything in the first dir
but dont ignore

The output of git status tells us which files should be searched.
I add it so i can also check that gitignores behaviour did not change.
  $ git status --short --untracked-files=all
  ?? ../.gitignore

The test
  $ ag test
  [1]


Make sure we are also able to override from parent
  $ echo "!subdir/b.txt" >> .gitignore
  $ mkdir subdir
  $ cd subdir
  $ echo "testb" > b.txt

  $ git status --short --untracked-files=all
  ?? ../../.gitignore
  ?? ../.gitignore
  ?? b.txt

  $ ag test
  b.txt:1:testb

