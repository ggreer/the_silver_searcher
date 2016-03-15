Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c
  $ printf 'whatever1\n'  > ./a/b/c/foo.yml
  $ printf 'whatever2\n'  > ./a/b/c/foo.yml~
  $ printf 'whatever3\n'  > ./a/b/c/.foo.yml.swp
  $ printf 'whatever4\n'  > ./a/b/c/.foo.yml.swo
  $ printf 'whatever5\n'  > ./a/b/foo.yml
  $ printf 'whatever6\n'  > ./a/b/foo.yml~
  $ printf 'whatever7\n'  > ./a/b/.foo.yml.swp
  $ printf 'whatever8\n'  > ./a/b/.foo.yml.swo
  $ printf 'whatever9\n'  > ./a/foo.yml
  $ printf 'whatever10\n' > ./a/foo.yml~
  $ printf 'whatever11\n' > ./a/.foo.yml.swp
  $ printf 'whatever12\n' > ./a/.foo.yml.swo
  $ printf 'whatever13\n' > ./foo.yml
  $ printf 'whatever14\n' > ./foo.yml~
  $ printf 'whatever15\n' > ./.foo.yml.swp
  $ printf 'whatever16\n' > ./.foo.yml.swo
  $ printf '*~\n'         > ./.gitignore
  $ printf '*.sw[po]\n'   >> ./.gitignore

Ignore all files except foo.yml

  $ ag whatever . | sort
  a/b/c/foo.yml:1:whatever1
  a/b/foo.yml:1:whatever5
  a/foo.yml:1:whatever9
  foo.yml:1:whatever13

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  .foo.yml.swo:1:whatever16
  .foo.yml.swp:1:whatever15
  a/.foo.yml.swo:1:whatever12
  a/.foo.yml.swp:1:whatever11
  a/b/.foo.yml.swo:1:whatever8
  a/b/.foo.yml.swp:1:whatever7
  a/b/c/.foo.yml.swo:1:whatever4
  a/b/c/.foo.yml.swp:1:whatever3
  a/b/c/foo.yml:1:whatever1
  a/b/c/foo.yml~:1:whatever2
  a/b/foo.yml:1:whatever5
  a/b/foo.yml~:1:whatever6
  a/foo.yml:1:whatever9
  a/foo.yml~:1:whatever10
  foo.yml:1:whatever13
  foo.yml~:1:whatever14
