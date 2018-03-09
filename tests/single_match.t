Setup:

  $ . $TESTDIR/setup.sh
  $ printf 'Hello, "Hello, world" programs output "Hello, world".\n' > ./test_single_match.txt
  $ printf '"Hello, world" programs are simple programs.\n' >> ./test_single_match.txt
  $ printf 'They illustrate the most basic syntax of a programming language\n' >> ./test_single_match.txt
  $ printf 'In javascript: alert("Hello, world!");\n' >> ./test_single_match.txt

Search for lines matching "hello" in test_single_match.txt (should only return first match):

  $ ag --single-match hello
  test_single_match.txt:1:Hello, "Hello, world" programs output "Hello, world".

Search for lines matching "hello" in test_single_match.txt (should only return first match):

  $ ag -1 hello
  test_single_match.txt:1:Hello, "Hello, world" programs output "Hello, world".