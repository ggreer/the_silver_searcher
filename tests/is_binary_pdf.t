Setup:

  $ . $TESTDIR/setup.sh
  $ cp $TESTDIR/is_binary.pdf .

PDF files are binary. Do not search them by default:

  $ ag PDF
  [1]

OK, search binary files

  $ ag --search-binary PDF
  file is_binary.pdf matches
