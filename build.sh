#!/bin/sh

aclocal && \
autoconf && \
autoheader && \
automake --add-missing && \
./configure && \
make
