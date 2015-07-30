Setup:

  $ . $TESTDIR/setup.sh
  $ echo "12345678901234567890123456789012345678901234567890" >> ./blah.txt

Truncate to width inside input line length:

  $ ag -W 20 1 < ./blah.txt
  blah.txt:1:12345678901234567890 [...]

Truncate to width outside input line length:

  $ ag -W 60 1 < ./blah.txt
  blah.txt:1:12345678901234567890123456789012345678901234567890

Truncate to width exactly input line length:

  $ ag -W 50 1 < ./blah.txt
  blah.txt:1:12345678901234567890123456789012345678901234567890

