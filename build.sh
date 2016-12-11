#!/bin/sh

set -e
cd "$(dirname "$0")"

./autogen.sh
./configure "$@"
make -j4
