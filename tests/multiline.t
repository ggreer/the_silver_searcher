Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'what' > blah.txt
  $ echo 'ever' >> blah.txt
  $ echo 'whatever' >> blah.txt

Multiline:

  $ ag 'wh[^w]+er' .
  blah.txt:1:what
  blah.txt:2:ever
  blah.txt:3:whatever

No multiline:

  $ ag --nomultiline 'wh[^w]+er' .
  blah.txt:3:whatever

Multiline explicit:

  $ ag '^wh[^w\n]+er$' .
  blah.txt:3:whatever
