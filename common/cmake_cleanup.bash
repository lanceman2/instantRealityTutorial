#!/bin/bash

scriptdir="$(dirname ${BASH_SOURCE[0]})"

set -x

cd "$scriptdir" || exit 1

rm -f\
 *.o\
 *.iio\
 CMakeCache.txt\
 cmake_install.cmake\
 Makefile || exit 1

rm -rf CMakeFiles/
