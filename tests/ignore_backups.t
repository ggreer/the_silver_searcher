Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p ./a/b/c
  $ echo 'whatever1'  > ./a/b/c/foo.yml
  $ echo 'whatever2'  > ./a/b/c/foo.yml~
  $ echo 'whatever3'  > ./a/b/c/.foo.yml.swp
  $ echo 'whatever4'  > ./a/b/c/.foo.yml.swo
  $ echo 'whatever5'  > ./a/b/foo.yml
  $ echo 'whatever6'  > ./a/b/foo.yml~
  $ echo 'whatever7'  > ./a/b/.foo.yml.swp
  $ echo 'whatever8'  > ./a/b/.foo.yml.swo
  $ echo 'whatever9'  > ./a/foo.yml
  $ echo 'whatever10' > ./a/foo.yml~
  $ echo 'whatever11' > ./a/.foo.yml.swp
  $ echo 'whatever12' > ./a/.foo.yml.swo
  $ echo 'whatever13' > ./foo.yml
  $ echo 'whatever14' > ./foo.yml~
  $ echo 'whatever15' > ./.foo.yml.swp
  $ echo 'whatever16' > ./.foo.yml.swo
  $ ( echo '*~'; echo '*.sw[po]' ) > ./.gitignore

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
