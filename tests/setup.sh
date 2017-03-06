#!/bin/bash

# All cram tests should use this. Make sure that "ag" runs the version
# of ag we just built, and make the output really simple.

# --noaffinity is to stop Travis CI from erroring (it runs in containers so pthread_setaffinity_np fails)
# --workers=1 is to keep all output ordered, to make testing output easier
# shellcheck disable=2139
alias ag="$TESTDIR/../ag --noaffinity --nocolor --workers=1 --parallel"
