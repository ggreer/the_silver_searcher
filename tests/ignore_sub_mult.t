Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b
  $ echo 'needle' > ./a/b/c
  $ echo 'a/b' > ./.gitignore

Ignore a/b/c:

  $ ag needle .

Dont ignore anything (unrestricted search):

  $ ag -u needle .
  a/b/c:1:needle

