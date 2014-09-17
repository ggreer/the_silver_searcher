Setup:

  $ . $TESTDIR/setup.sh
  $ mkdir -p foo/bar/baz
  $ echo needle > foo/quux
  $ echo needle > foo/bar/quux
  $ echo needle > foo/bar/baz/quux

Ignore foo:

  $ ag needle --ignore foo
  [1]

Ignore foo/bar:

  $ ag needle --ignore foo/bar
  foo/quux:1:needle

Ignore foo/bar/baz:

  $ ag needle --ignore foo/bar/baz | sort
  foo/bar/quux:1:needle
  foo/quux:1:needle

Ignore nonexistent dir:

  $ ag needle --ignore nothing | sort
  foo/bar/baz/quux:1:needle
  foo/bar/quux:1:needle
  foo/quux:1:needle

Ignore nothing:

  $ ag needle | sort
  foo/bar/baz/quux:1:needle
  foo/bar/quux:1:needle
  foo/quux:1:needle
