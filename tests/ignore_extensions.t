Setup:

  $ . $TESTDIR/setup.sh
  $ printf '*.js\n' > .ignore
  $ printf '*.test.txt\n' >> .ignore
  $ printf 'targetA\n' > something.js
  $ printf 'targetB\n' > aFile.test.txt
  $ printf 'targetC\n' > aFile.txt
  $ printf 'targetG\n' > something.min.js
  $ mkdir -p subdir
  $ printf 'targetD\n' > subdir/somethingElse.js
  $ printf 'targetE\n' > subdir/anotherFile.test.txt
  $ printf 'targetF\n' > subdir/anotherFile.txt
  $ printf 'targetH\n' > subdir/somethingElse.min.js
  $ printf 'targetI\n' > subdir/anotherFile.some.test.txt

Ignore patterns with single extension in root directory:

  $ ag "targetA"
  [1]

Ignore patterns with multiple extensions in root directory:

  $ ag "targetB"
  [1]

*.js ignores *.min.js in root directory:

  $ ag "targetG"
  [1]

Do not ignore patterns with partial extensions in root directory:

  $ ag "targetC"
  aFile.txt:1:targetC

Ignore patterns with single extension in subdirectory:

  $ ag "targetD"
  [1]

Ignore patterns with multiple extensions in subdirectory:

  $ ag "targetE"
  [1]

*.js ignores *.min.js in subdirectory:

  $ ag "targetH"
  [1]

Do not ignore patterns with partial extensions in subdirectory:

  $ ag "targetF"
  subdir/anotherFile.txt:1:targetF

*.test.txt ignores *.some.test.txt

  $ ag "targetI"
  [1]
