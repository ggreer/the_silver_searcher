Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --nocolor --workers=1"

Print context when searching stdin:

  $ echo "before1\nbefore2\nblah\nafter1\nafter2" | ag -C blah
  before1
  before2
  blah
  after1
  after2

  $ echo "before1\nbefore2\nbefore3\nblah\nbetween1\nblah\nafter1" | ag -C blah
  before2
  before3
  blah
  between1
  blah
  after1

Print only a line before when searching stdin:

  $ echo "before1\nbefore2\nblah\nafter1\nafter2" | ag -B1 blah
  before2
  blah

Print only a line after when searching stdin:

  $ echo "before1\nbefore2\nblah\nafter1\nafter2" | ag -A1 blah
  blah
  after1
