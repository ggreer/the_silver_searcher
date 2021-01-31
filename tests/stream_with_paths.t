Setup:

  $ . $TESTDIR/setup.sh
  $ unalias ag
  $ alias ag="$TESTDIR/../ag --noaffinity --nocolor --workers=1"
  $ echo foo > ./stream_with_path.txt
  $ echo foo > ./stream_with_path_2.txt
  $ echo bar > ./stream_with_path_3.txt

Search for "foo" in paths passed as arguments:

  $ echo | ag foo stream_with_path.txt stream_with_path_3.txt
  stream_with_path.txt:1:foo

  $ echo | ag --nofilename foo stream_with_path.txt stream_with_path_3.txt
  foo

  $ echo | ag --nonumbers foo stream_with_path.txt stream_with_path_3.txt
  stream_with_path.txt:foo
