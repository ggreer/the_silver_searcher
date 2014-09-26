Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c ./d/e/f ./g/a/b ./h/d/e
  $ echo 'whatever1' > ./a/b/c/foo.yml
  $ echo 'whatever2' > ./d/e/f/foo.yml
  $ echo 'whatever3' > ./g/a/b/foo.yml
  $ echo 'whatever4' > ./h/d/e/foo.yml
  $ echo 'a/b\n/d/e' > ./.gitignore

Ignore files in ./a/b or ./d/e

  $ ag whatever . | sort
  g/a/b/foo.yml:1:whatever3
  h/d/e/foo.yml:1:whatever4

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  a/b/c/foo.yml:1:whatever1
  d/e/f/foo.yml:1:whatever2
  g/a/b/foo.yml:1:whatever3
  h/d/e/foo.yml:1:whatever4
