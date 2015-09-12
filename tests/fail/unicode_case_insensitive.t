Setup:

  $ . $TESTDIR/../setup.sh
  $ echo "hello=你好" > test.txt
  $ echo "hello=你好" >> test.txt

Normal search:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel 你好
  test.txt:1:hello=你好
  test.txt:2:hello=你好

Case-insensitive search:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel -i 你好
  test.txt:1:hello=你好
  test.txt:2:hello=你好
