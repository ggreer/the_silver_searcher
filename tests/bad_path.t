Setup:

  $ . $TESTDIR/setup.sh

Complain about nonexistent path:

  $ ag foo doesnt_exist
  ERR: Error stat()ing: doesnt_exist
  ERR: Error opening directory doesnt_exist: No such file or directory
