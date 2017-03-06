#!/bin/sh

set -e
cd "$(dirname "$0")"

make clean
CFLAGS="$CFLAGS -fprofile-generate"
./build.sh
./ag example ..
make clean
CFLAGS="$CFLAGS -fprofile-use"
./build.sh
