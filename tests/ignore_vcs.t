Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'whatever1\n' > ./always.txt
  $ printf 'whatever2\n' > ./git.txt
  $ printf 'whatever3\n' > ./text.txt
  $ printf 'git.txt\n' > ./.gitignore
  $ printf 'text.*\n' > ./.ignore

Obey .gitignore and .ignore patterns:

  $ ag whatever .
  always.txt:1:whatever1

Ignore .gitignore patterns but not .ignore patterns:

  $ ag -U whatever . | sort
  always.txt:1:whatever1
  git.txt:1:whatever2
