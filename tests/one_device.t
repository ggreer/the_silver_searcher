Setup:

  $ . $TESTDIR/setup.sh
  > if [ ! -e "/dev/shm" ]; then
  > echo "No /dev/shm. Skipping test."
  > exit 80
  > elif [ "$(stat -c%d /dev/)" == "$(stat -c%d /dev/shm/)" ]; then
  > echo "/dev/shm not a different device.  Skipping test."
  > exit 80
  > fi
  $ TEST_TMPDIR=`mktemp -d --tmpdir=/dev/shm ag_test.XXX`
  $ echo "blah" > $TEST_TMPDIR/blah.txt
  $ ln -s $TEST_TMPDIR other_device

Should not descend into /dev/shm symlink when --one-device specified:

  $ ag -f --one-device blah .
  [1]

Files on other devices work the same way as anything else without --one-device:

  $ ag -f blah .
  other_device/blah.txt:1:blah

Cleanup:
  $ rm -r $TEST_TMPDIR
