Setup:

  $ . $TESTDIR/setup.sh
  $ printf "hello world\n" >test.txt

Verify ag runs with an empty environment:

  $ env -i $TESTDIR/../ag --noaffinity --nocolor --workers=1 --parallel hello
  test.txt:1:hello world
