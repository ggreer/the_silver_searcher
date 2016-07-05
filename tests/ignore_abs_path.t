Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c
  $ printf 'whatever1\n' > ./a/b/c/blah.yml
  $ printf 'whatever2\n' > ./a/b/foo.yml
  $ printf '/a/b/foo.yml\n' > ./.gitignore

Ignore foo.yml but not blah.yml:

  $ ag whatever .
  a/b/c/blah.yml:1:whatever1

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  WARN: Hidden directories searchable with current flag.
  a/b/c/blah.yml:1:whatever1
  a/b/foo.yml:1:whatever2

Ignore foo.yml given an absolute search path [#448]:

  $ ag whatever $(pwd)
  /.*/a/b/c/blah.yml:1:whatever1 (re)
