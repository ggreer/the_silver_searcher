Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'what\n' > blah.txt
  $ printf 'ever\n' >> blah.txt
  $ printf 'whatever\n' >> blah.txt

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
