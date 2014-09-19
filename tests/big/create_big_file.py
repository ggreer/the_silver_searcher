#!/usr/bin/env python

# Create an 8GB file of mostly "abcdefghijklmnopqrstuvwxyz01234",
# with a few instances of "hello"

import sys

if len(sys.argv) != 2:
    print("Usage: %s big_file.txt" % sys.argv[0])
    sys.exit(1)

big_file = sys.argv[1]


def create_big_file():
    with open(big_file, "w") as fd:
        for i in range(1, 2**28):
            byte = i * 32
            if byte % 2**30 == 0:
                fd.write("hello%s\n" % byte)
            else:
                fd.write("abcdefghijklmnopqrstuvwxyz01234\n")
        fd.write("hello\n")


try:
    fd = open(big_file, "r")
except Exception as e:
    create_big_file()
