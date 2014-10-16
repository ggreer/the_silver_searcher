Setup:

  $ . $TESTDIR/setup.sh
  $ echo "foo bar" > files_from_test_1.txt
  $ echo "zoo zar" >> files_from_test_2.txt
  $ echo "foo test zoo" >> files_from_test_3.a
  $ echo "zoo zoo" >> files_from_test_4.txt
  $ echo "files_from_test_1.txt" > files.txt
  $ echo "files_from_test_2.txt" >> files.txt
  $ echo "files_from_test_3.a" >> files.txt
  $ echo "*.txt" > agignore

test files-from on command line, and make sure ignore files are, well, ignored:

  $ ag --path-to-agignore agignore --files-from files.txt zoo 
  files_from_test_2.txt:1:zoo zar
  files_from_test_3.a:1:foo test zoo

Just to make sure the ignore file works normally:

  $ ag --path-to-agignore agignore zoo
  files_from_test_3.a:1:foo test zoo

Test from stdin via cat:

  $ cat files.txt | ag -x zoo
  files_from_test_2.txt:1:zoo zar
  files_from_test_3.a:1:foo test zoo

Test from stdin via shell redirection:

  $ ag -x zar < files.txt
  files_from_test_2.txt:1:zoo zar
