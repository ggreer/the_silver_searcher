Setup:

  $ . $TESTDIR/../setup.sh
  $ printf "hello=你好\n" > test.txt
  $ printf "hello=你好\n" >> test.txt

Normal search:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel 你好
  test.txt:1:hello=你好
  test.txt:2:hello=你好

Case-insensitive search:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel -i 你好
  test.txt:1:hello=你好
  test.txt:2:hello=你好
