
Setup:
  $ . $TESTDIR/setup.sh
  $ echo "This should always get recognized #is#a"      >> ./sample
  $ echo "This# should never get recognized #isit?"     >> ./sample
  $ echo "Is OK if start with pattern?"                 >> ./sample
  $ echo "Finishing with pattern also OK is"            >> ./sample


Literals work porperly:

  $ ag is sample -Qw
  1:This should always get recognized #is#a
  3:Is OK if start with pattern?
  4:Finishing with pattern also OK is

Works for simple regexes:

  $ ag is sample -w
  1:This should always get recognized #is#a
  3:Is OK if start with pattern?
  4:Finishing with pattern also OK is

  $ ag '\w{2}' sample -w
  1:This should always get recognized #is#a
  3:Is OK if start with pattern?
  4:Finishing with pattern also OK is

Regexes properly recognize partial word boundaries:

  $ ag 'is#' sample -w
  1:This should always get recognized #is#a

  $ ag '#is' sample -w
  1:This should always get recognized #is#a

Works on multi-word regexes:

  $ ag 'always g\w{2} recog.*zed' sample
  1:This should always get recognized #is#a

It fails to match unless on a word boundary:

  $ ag 'sh' sample -w
  [1]

  $ ag 'ou' sample -w
  [1]

  $ ag 'ld' sample -w
  [1]
