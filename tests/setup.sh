#!/bin/bash

# All cram tests should use this. Make sure that "ag" runs the version
# of ag we just built, and make the output really simple.

# shellcheck disable=2139
alias ag="$TESTDIR/../ag --nocolor --workers=1 --parallel"
