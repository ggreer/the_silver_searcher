#!/bin/sh

aclocal && \
automake && \
./configure && \
make
