Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c
  $ echo 'whatever1' > ./a/b/c/blah.yml
  $ echo 'whatever2' > ./a/b/foo.yml
  $ echo 'a/b/foo.yml' > ./.gitignore
# TODO: have this work instead of the above
# $ echo 'a/b/*.yml' > ./.gitignore

Ignore foo.yml but not blah.yml:

  $ ag whatever .
  a/b/c/blah.yml:1:whatever1

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  a/b/c/blah.yml:1:whatever1
  a/b/foo.yml:1:whatever2
