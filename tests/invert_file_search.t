Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'hello' > ./some_file_abc_123
  $ printf 'hello' > ./some_file_def_123

Find both matches normally:

  $ ag hello
  some_file_abc_123:1:hello
  some_file_def_123:1:hello

Find only the def match when we block abc

  $ ag hello --invert-file-search-regex abc
  some_file_def_123:1:hello

Find only the abc match when we block def

  $ ag hello --invert-file-search-regex def
  some_file_abc_123:1:hello

Find no matches when we block everything

  $ ag hello --invert-file-search-regex ".*"
  [1]
