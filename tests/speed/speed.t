Setup and create really big file:

  $ . $TESTDIR/../setup.sh
  $ 

Search a big file:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel hello $TESTDIR/many_dirs
