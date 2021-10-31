Setup:

  $ . $TESTDIR/setup.sh
  $ alias ag="$TESTDIR/../ag --noaffinity --nocolor --workers=1"
  $ printf bla > ag-input.txt

Terminating null byte isn't printed for lines which don't end with a newline:

  $ ag --vimgrep bla
  ag-input.txt:1:1:bla
  $ ag --ackmate bla
  :ag-input.txt
  1;0 3:bla
