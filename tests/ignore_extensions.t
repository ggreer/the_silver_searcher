Setup:

  $ . $TESTDIR/setup.sh
  $ printf '*.js\n' > .ignore
  $ printf '*.test.txt\n' >> .ignore
  $ printf 'targetA\n' > something.js
  $ printf 'targetB\n' > aFile.test.txt
  $ printf 'targetC\n' > aFile.txt
  $ mkdir -p subdir
  $ printf 'targetD\n' > subdir/somethingElse.js
  $ printf 'targetE\n' > subdir/anotherFile.test.txt
  $ printf 'targetF\n' > subdir/anotherFile.txt

Ignore patterns with single extension in root directory:

  $ ag "targetA"
  [1]

Ignore patterns with multiple extensions in root directory:

  $ ag "targetB"
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

Do not ignore patterns with partial extensions in subdirectory:

  $ ag "targetF"
  subdir/anotherFile.txt:1:targetF
