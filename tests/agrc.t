Setup:

  $ . $TESTDIR/setup.sh
  $ alias ag="$TESTDIR/../ag --noaffinity --nocolor --workers=1 --parallel"
  $ printf -- '-s\n--ignore=foo.*\n' >agrc
  $ printf 'hello\nHello\n' >file.txt
  $ printf 'hello\n' >foo.txt

agrc enables case-sensitive matching and ignores foo.txt:

  $ ag --agrc=agrc hello
  file.txt:1:hello
