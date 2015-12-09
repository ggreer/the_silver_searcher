Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/bomb
  $ echo 'whatever' > ./a/bomb/foo.yml
  $ echo '*b/foo.yml' > ./.gitignore

Ignore foo.yml but not blah.yml:

  $ ag whatever .
  [1]

Dont ignore anything (unrestricted search):

  $ ag -u whatever .
  a/bomb/foo.yml:1:whatever
