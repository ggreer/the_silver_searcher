Setup:

  $ . $TESTDIR/setup.sh
  $ echo main > ./main.c
  $ echo sample > ./sample.c
  $ echo main sample > ./sample.h

Filename matches work

  $ ag -g '\.c'
  main.c
  sample.c
  $ ag main | sort
  main.c:1:main
  sample.h:1:main sample
  $ ag -G '\.c' main
  main.c:1:main

Inverse filename matches work

  $ ag -g '!\.c'
  sample.h
  $ ag -G '!\.c' main
  sample.h:1:main sample
