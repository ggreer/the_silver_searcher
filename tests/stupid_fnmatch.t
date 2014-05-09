Setup:

  $ source $TESTDIR/setup.sh
  $ mkdir -p ./a/bomb ./b ./cab
  $ echo 'whatever1' > ./a/bomb/foo.yml
  $ echo 'whatever2' > ./b/foo.yml
  $ echo 'whatever3' > ./cab/foo.yml
  $ echo '*b/foo.yml' > ./.gitignore

Ignore b/foo.yml and cab/foo.yml but not a/bomb/foo.yml:

  $ ag whatever .
  a/bomb/foo.yml:1:whatever1

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  a/bomb/foo.yml:1:whatever1
  b/foo.yml:1:whatever2
  cab/foo.yml:1:whatever3
