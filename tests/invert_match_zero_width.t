Setup:

  $ . $TESTDIR/setup.sh

An inverted zero-width pattern was (in the past) causing ag to loop due to
integer underflow.

First tests are not a problem
  $ echo "test...just one line" > ./blah.txt
  $ ag -v '^$' blah.txt
  1:test...just one line

  $ echo "kilroy  ............" > ./blah.txt
  $ echo "end of working stuff" >> ./blah.txt
  $ ag -v '^$' blah.txt
  1:kilroy  ............
  2:end of working stuff

Now come the problem children...start with an empty file
  $ echo                 > ./blah.txt
  $ ag -v '^$' blah.txt
  [1]

  $ echo                 > ./blah.txt
  $ echo  'add a line'  >> ./blah.txt
  $ ag -v '^$' blah.txt
  2:add a line

  $ echo  'add a line'   > ./blah.txt
  $ echo                >> ./blah.txt
  $ ag -v '^$' blah.txt
  1:add a line

  $ echo                 > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'zero zero'   >> ./blah.txt
  $ ag -v '^$' blah.txt
  3:zero zero

  $ echo                 > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'zero zero 0' >> ./blah.txt
  $ ag -v '^$' blah.txt
  4:zero zero 0

  $ echo                 > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'zero zero 0' >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'sludge'      >> ./blah.txt
  $ ag -v '^$' blah.txt
  4:zero zero 0
  7:sludge

  $ echo  'zero zero'    > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo                >> ./blah.txt
  $ ag -v '^$' blah.txt
  1:zero zero

  $ echo  'add a line'   > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'end line  '  >> ./blah.txt
  $ ag -v '^$' blah.txt
  1:add a line
  3:end line  

  $ echo  'L'           > ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'mm'          >> ./blah.txt
  $ echo                >> ./blah.txt
  $ ag -v '^$' blah.txt
  1:L
  3:mm

  $ echo                > ./blah.txt
  $ echo  'Y'           >> ./blah.txt
  $ echo  'Z'           >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'X'           >> ./blah.txt
  $ ag -v '^$' blah.txt
  2:Y
  3:Z
  5:X

  $ echo                 > ./blah.txt
  $ echo  'middle line' >> ./blah.txt
  $ echo                >> ./blah.txt
  $ ag -v '^$' blah.txt
  2:middle line

  $ echo                 > ./blah.txt
  $ echo  'second line' >> ./blah.txt
  $ echo                >> ./blah.txt
  $ echo  'fine (30) '  >> ./blah.txt
  $ ag -v '^$' blah.txt
  2:second line
  4:fine (30) 

