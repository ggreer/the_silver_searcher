Setup:

  $ . $TESTDIR/setup.sh
  $ printf '#!/bin/sh\n' > scan1.bash
  $ printf 'foo 1 32000 1 \\\n' >> scan1.bash
  $ printf '#!/bin/sh\n' > scan2.bash
  $ printf 'foo 1 32000 1 \\\n' >> scan2.bash

Print no matches:

  $ ag -L "/bin/sh" scan1.bash scan2.bash
  [1]
