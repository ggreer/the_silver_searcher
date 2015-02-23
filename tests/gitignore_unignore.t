Setup:

  $ . $TESTDIR/setup.sh
  $ git init --quiet
  $ echo 'a.txt' > .gitignore
  $ echo "test" > a.txt


The test
  $ ag test
  [1]

  $ echo '!a.txt' >> .gitignore

The output of git status tells us which files should be searched.
I add it so i can also check that gitignores behaviour did not change.
  $ git status --short --untracked-files=all
  ?? .gitignore
  ?? a.txt

  $ ag test
  a.txt:1:test



