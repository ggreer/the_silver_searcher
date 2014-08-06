Setup and create really big file:

  $ . $TESTDIR/../setup.sh
  $ python3 $TESTDIR/create_big_file.py $TESTDIR/big_file.txt

Search a big file:

  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel hello $TESTDIR/big_file.txt
  33554432:hello1073741824
  67108864:hello2147483648
  100663296:hello3221225472
  134217728:hello4294967296
  167772160:hello5368709120
  201326592:hello6442450944
  234881024:hello7516192768
  268435456:hello

Fail to regex search a big file:
  $ $TESTDIR/../../ag --nocolor --workers=1 --parallel 'hello.*' $TESTDIR/big_file.txt
  ERR: Skipping */big_file.txt: pcre_exec() can't handle files larger than 2147483647 bytes. (glob)
  [1]
