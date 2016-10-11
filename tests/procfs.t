Setup:

  $ . "${TESTDIR}/setup.sh"

Should support searching procfs on Linux.

  $ test_procfs() {
  >   # This test only applies to Linux — skip it if we don't have that
  >   uname | grep -qiF 'linux' || exit 80
  >   # Can't see how this would happen, but just in case
  >   grep -qsE '[0-9]' /proc/uptime || exit 80
  >   # /proc/uptime on Linux looks like this: 532030.52 1054207.60
  >   ag '^[\d.]+\s+[\d.]+$' /proc/uptime
  >   return 0
  > }
  $ test_procfs
  1:[\d.]+\s+[\d.]+ (re)
