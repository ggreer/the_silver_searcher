Setup:

  $ . $TESTDIR/setup.sh
  $ export HOME=$PWD
  $ printf '[core]\nexcludesfile = ~/.gitignore.global' >> $HOME/.gitconfig
  $ printf '*\n!PATTERN_MARKER\n' > .gitignore.global
  $ printf 'testing1234' > PATTERN_MARKER

Test that the unignore works correctly:

  $ ag -l . --debug | grep "PATTERN_MARKER.*Unignore match?: true"
  DEBUG: pattern * doesn't match file PATTERN_MARKER.  Unignore match?: true
