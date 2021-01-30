Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c
  $ printf 'whatever1\n' > ./a/b/c/blah.yml
  $ printf 'whatever2\n' > ./a/b/foo.yml
  $ printf 'a/b/*.yml\n' > ./.gitignore

Ignore foo.yml but not blah.yml:

  $ ag whatever .
  a/b/c/blah.yml:1:whatever1

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  a/b/c/blah.yml:1:whatever1
  a/b/foo.yml:1:whatever2
