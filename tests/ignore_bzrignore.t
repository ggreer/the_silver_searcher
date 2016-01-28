Setup:

  $ . $TESTDIR/setup.sh
  $ export HOME=$PWD
  $ echo 'PATTERN_MARKER' > .bzrignore

Test that the ignore pattern got picked up:

  $ ag --debug . | grep PATTERN_MARKER
  DEBUG: added ignore pattern PATTERN_MARKER to 

