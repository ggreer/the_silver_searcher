Setup:

  $ . $TESTDIR/setup.sh
  $ printf "12345678901234567890123456789012345678901234567890\n" >> ./blah.txt

Truncate to width inside input line length:

  $ ag -W 20 1 < ./blah.txt
  blah.txt:1:12345678901234567890 [...]

Truncate to width inside input line length, long-form:

  $ ag --width 20 1 < ./blah.txt
  blah.txt:1:12345678901234567890 [...]

Truncate to width outside input line length:

  $ ag -W 60 1 < ./blah.txt
  blah.txt:1:12345678901234567890123456789012345678901234567890

Truncate to width one less than input line length:

  $ ag -W 49 1 < ./blah.txt
  blah.txt:1:1234567890123456789012345678901234567890123456789 [...]

Truncate to width exactly input line length:

  $ ag -W 50 1 < ./blah.txt
  blah.txt:1:12345678901234567890123456789012345678901234567890

