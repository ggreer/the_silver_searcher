Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'asdf\n' > test.txt
  $ printf 'AsDf\n' >> test.txt

Smart case search:

  $ ag -S asdf -G "test.txt"
  test.txt:1:asdf
  test.txt:2:AsDf

Order of options should not matter:

  $ ag asdf -G "test.txt" -S 
  test.txt:1:asdf
  test.txt:2:AsDf
