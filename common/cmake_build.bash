#!/bin/bash

scriptdir="$(dirname ${BASH_SOURCE[0]})"

set -x

cd "$scriptdir" || exit 1

if [ ! -d CMakeFiles ] ; then
  cmake\
 -G"Unix Makefiles"\
 -DCMAKE_CXX_FLAGS:STRING="-g -Wall -Werror"\
 || exit 1
fi

make VERBOSE=1

