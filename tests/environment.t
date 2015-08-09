Setup:

  $ . $TESTDIR/setup.sh
  $ echo Foo > ./sample
  $ echo bar >> ./sample
  $ echo baz >> ./sample

Context (int) from AG_CONTEXT

  $ AG_CONTEXT=1 ag bar sample
  1-Foo
  2:bar
  3-baz

Before (size_t) from AG_BEFORE

  $ AG_BEFORE=1 ag bar sample
  1-Foo
  2:bar

Line numbers (boolean, false) from AG_LINE_NUMBERS

  $ AG_LINE_NUMBERS=false ag bar sample
  bar

Whole word matching (boolean, true) from AG_WHOLE_WORD

  $ echo barr >> ./sample
  $ AG_WHOLE_WORD=1 ag bar sample
  2:bar

Case matching (string) from AG_CASE

  $ AG_CASE=insensitive ag fOO sample
  1:Foo

Match color from AG_COLOR_MATCH

  $ AG_COLOR_MATCH=30\;41 ag --color --no-numbers Foo sample
  \x1b[30;41mFoo\x1b[0m\x1b[K (esc)
