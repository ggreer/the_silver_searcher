Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'foo\nあいうえお\n' | iconv -f utf-8 -t sjis > ./sample_shift_jis
  $ printf 'foo\nあいうえお\n' | iconv -f utf-8 -t euc-jp > ./sample_euc_jp

Files with Japanese encoding:

  $ ag foo sample_shift_jis
  1:foo
  $ ag foo sample_euc_jp
  1:foo

