Setup:

  $ . $TESTDIR/setup.sh
  $ git init --quiet
  $ mkdir -p a/1
  $ mkdir -p b/1
  $ mkdir -p c/1
  $ mkdir -p d/1
  $ mkdir -p e/1
  $ echo 'test' > ./a/a.txt
  $ echo 'test' > ./a/1/a.txt
  $ echo 'test' > ./b/a.txt
  $ echo 'test' > ./c/a.txt
  $ echo 'test' > ./d/a.txt
  $ echo 'test' > ./e/a.txt
  $ echo "test" > a.txt

Make sure we ignore everything in the first dir
but dont ignore /*/1
  $ echo "/*/a.txt" > .gitignore

The output of git status tells us which files should be searched.
I add it so i can also check that gitignores behaviour did not change.
  $ git status --short --untracked-files=all
  ?? .gitignore
  ?? a.txt
  ?? a/1/a.txt


The test
  $ ag test | sort
  a.txt:1:test
  a/1/a.txt:1:test

  $ echo "!/*/a.txt" >> .gitignore
  $ echo "/*/1/a.txt" >> .gitignore

  $ git status --short --untracked-files=all
  ?? .gitignore
  ?? a.txt
  ?? a/a.txt
  ?? b/a.txt
  ?? c/a.txt
  ?? d/a.txt
  ?? e/a.txt

  $ ag test | sort
  a.txt:1:test
  a/a.txt:1:test
  b/a.txt:1:test
  c/a.txt:1:test
  d/a.txt:1:test
  e/a.txt:1:test


Now test it without slash at the beginning

  $ echo 'test' > ./b/1/a.txt
  $ echo 'test' > ./c/1/a.txt
  $ echo 'test' > ./d/1/a.txt
  $ echo 'test' > ./e/1/a.txt

  $ echo '*/a.txt' > .gitignore
  $ echo 'a.txt' >> .gitignore
  $ echo '!*/1/a.txt' >> .gitignore

  $ git status --untracked-files=all --short
  ?? .gitignore
  ?? a/1/a.txt
  ?? b/1/a.txt
  ?? c/1/a.txt
  ?? d/1/a.txt
  ?? e/1/a.txt

  $ ag test | sort
  a/1/a.txt:1:test
  b/1/a.txt:1:test
  c/1/a.txt:1:test
  d/1/a.txt:1:test
  e/1/a.txt:1:test
