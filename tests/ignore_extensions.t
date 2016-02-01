Setup:

  $ . $TESTDIR/setup.sh
  $ echo '*.js' > .gitignore
  $ echo '\n*.test.txt' >> .gitignore
  $ echo 'targetA' > something.js
  $ echo 'targetG' > something.min.js
  $ echo 'targetB' > aFile.test.txt
  $ echo 'targetC' > aFile.txt
  $ mkdir -p subdir
  $ echo 'targetD' > subdir/somethingElse.js
  $ echo 'targetH' > subdir/somethingElse.min.js
  $ echo 'targetE' > subdir/anotherFile.test.txt
  $ echo 'targetF' > subdir/anotherFile.txt

Ignore patterns with single extension in root directory:

  $ ag "targetA"
  [1]

Ignore patterns with multiple extensions in root directory:

  $ ag "targetB"
  [1]

Ignore files with multiple extensions in root directory:

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

Ignore files with multiple extensions in subdirectory:

  $ ag "targetH"
  [1]

Do not ignore patterns with partial extensions in subdirectory:

  $ ag "targetF"
  subdir/anotherFile.txt:1:targetF
