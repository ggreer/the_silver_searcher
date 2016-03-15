Setup:

  $ . $TESTDIR/setup.sh
  $ export HOME=$PWD
  $ printf '[core]\nexcludesfile = ~/.gitignore.global' >> $HOME/.gitconfig
  $ printf 'PATTERN_MARKER\n' > .gitignore.global

Test that the ignore pattern got picked up:

  $ ag --debug . | grep PATTERN_MARKER
  DEBUG: added ignore pattern PATTERN_MARKER to root ignores

