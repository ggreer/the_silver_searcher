Setup:

# This is a test for gitignores, where you can ignore all subcontent
# of a dir, but add exception to that.

$ . $TESTDIR/setup.sh
$ mkdir -p ./a/b
$ mkdir -p ./a/c
$ echo 'whatever1' > ./a/foo.txt
$ echo 'whatever2' > ./a/important.txt
$ echo 'whatever3' > ./a/b/bar.txt
$ echo 'whatever4' > ./a/c/baz.txt
$ echo 'a/*' >> ./.gitignore
$ echo '!a/important.txt' >> ./.gitignore

Ignore all but /a/important.txt.

  $ ag whatever . | sort
  a/important.txt:1:whatever2


Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  a/b/bar.txt:1:whatever3
  a/c/baz.txt:1:whatever4
  a/foo.txt:1:whatever1
  a/important.txt:1:whatever2
