Setup:

  $ source $TESTDIR/setup.sh
  $ mkdir -p ./.hidden
  $ echo 'whatever' > ./.hidden/a.txt

Ignore a.txt

  $ ag whatever .

Do not ignore a.txt

  $ ag --hidden whatever .
  .hidden/a.txt:1:whatever

