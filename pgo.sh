#!/bin/sh

set -e
cd "$(dirname "$0")"

make clean
./build.sh CFLAGS="$CFLAGS -fprofile-generate"
./ag example ..
make clean
./build.sh CFLAGS="$CFLAGS -fprofile-correction -fprofile-use"
