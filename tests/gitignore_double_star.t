Setup:

  $ . $TESTDIR/setup.sh
  $ git init --quiet
  $ echo "/**/a.txt" > .gitignore
  $ mkdir -p a/b/c/d/e/f
  $ echo 'test' > ./a/a.txt
  $ echo 'test' > ./a/b/a.txt
  $ echo 'test' > ./a/b/c/a.txt
  $ echo 'test' > ./a/b/c/d/a.txt
  $ echo 'test' > ./a/b/c/d/e/a.txt
  $ echo 'test' > ./a/b/c/d/e/f/a.txt
  $ echo "test" > a.txt


The output of git status tells us which files should be searched.
I add it so i can also check that gitignores behaviour did not change.
  $ git status --short --untracked-files=all
  ?? .gitignore

The test
  $ ag test
  [1]

  $ echo "!/a/b/c/**/a.txt" >> .gitignore

  $ git status --short --untracked-files=all
  ?? .gitignore
  ?? a/b/c/a.txt
  ?? a/b/c/d/a.txt
  ?? a/b/c/d/e/a.txt
  ?? a/b/c/d/e/f/a.txt

  $ ag test | sort
  a/b/c/a.txt:1:test
  a/b/c/d/a.txt:1:test
  a/b/c/d/e/a.txt:1:test
  a/b/c/d/e/f/a.txt:1:test

  $ rm -rf .git
  $ ag -u test | sort
  a.txt:1:test
  a/a.txt:1:test
  a/b/a.txt:1:test
  a/b/c/a.txt:1:test
  a/b/c/d/a.txt:1:test
  a/b/c/d/e/a.txt:1:test
  a/b/c/d/e/f/a.txt:1:test

