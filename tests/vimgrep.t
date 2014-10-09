Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'Hello, "Hello, world" programs output "Hello, world".' > ./test_vimgrep.txt
  $ echo '"Hello, world" programs are simple programs.' >> ./test_vimgrep.txt
  $ echo 'They illustrate the most basic syntax of a programming language' >> ./test_vimgrep.txt
  $ echo 'In javascript: alert("Hello, world!");' >> ./test_vimgrep.txt

Search for lines matching "hello" in test_vimgrep.txt:

  $ ag --vimgrep hello
  test_vimgrep.txt:1:1:Hello, "Hello, world" programs output "Hello, world".
  test_vimgrep.txt:1:9:Hello, "Hello, world" programs output "Hello, world".
  test_vimgrep.txt:1:40:Hello, "Hello, world" programs output "Hello, world".
  test_vimgrep.txt:2:2:"Hello, world" programs are simple programs.
  test_vimgrep.txt:4:23:In javascript: alert("Hello, world!");
