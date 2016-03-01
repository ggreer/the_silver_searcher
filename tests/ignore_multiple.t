Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'whatever1' > ./a.txt
  $ echo 'whatever2' > ./B.txt
  $ echo 'whatever3' > ./c.txt
  $ echo 'whatever4' > ./D.txt
  $ echo 'whatever5' > ./e.txt
  $ echo 'whatever6' > ./F.txt
  $ echo 'whatever7' > ./g.txt
  $ echo 'whatever8' > ./H.txt
  $ echo 'whatever9' > ./i.txt
  $ echo "B.txt\ne.txt\nH.txt" > ./.gitignore

Ignore B.txt, e.txt, and H.txt:

  $ ag whatever . | sort
  D.txt:1:whatever4
  F.txt:1:whatever6
  a.txt:1:whatever1
  c.txt:1:whatever3
  g.txt:1:whatever7
  i.txt:1:whatever9

Dont ignore anything (unrestricted search):

  $ ag -u whatever . | sort
  B.txt:1:whatever2
  D.txt:1:whatever4
  F.txt:1:whatever6
  H.txt:1:whatever8
  a.txt:1:whatever1
  c.txt:1:whatever3
  e.txt:1:whatever5
  g.txt:1:whatever7
  i.txt:1:whatever9
