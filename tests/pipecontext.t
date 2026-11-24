Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir pipecontext_dir
  $ printf "a\nb\nc\n" > pipecontext_test.txt
  $ cd pipecontext_dir

Do not use parallel flag, which disables stream input:

  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"

B flag on pipe:

  $ cat ../pipecontext_test.txt | ag --numbers -B1 b
  1-a
  2:b

C flag on pipe:

  $ cat ../pipecontext_test.txt | ag --numbers -C1 b
  1-a
  2:b
  3-c

Just match last line:

  $ cat ../pipecontext_test.txt | ag --numbers c
  3:c

Print a whole matching line at EOF even if the input has no terminating newline:

  $ printf 'one\ntwo' | ag --numbers two
  2:two

Keep a single terminator for individual matches and truncated lines:

  $ printf 'two' | ag -o two
  two
  $ printf 'twomore' | ag -W 3 two
  two [...]
