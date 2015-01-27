#!/bin/sh

# ronn is used to turn the markdown into a manpage.
# Get ronn at https://github.com/rtomayko/ronn
# Alternately, since ronn is a Ruby gem, you can just
# `gem install ronn`

sed -e 's/\\0/\\\\0/' <ag.1.md >ag.1.md.tmp
ronn -r ag.1.md.tmp

rm -f ag.1.md.tmp
