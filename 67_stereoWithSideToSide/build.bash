#!/bin/bash

scriptdir="$(dirname ${BASH_SOURCE[0]})"

set -x

cd "$scriptdir" || exit 1

if [ ! -d CMakeFiles ] ; then
  cmake . || exit 1
fi

make

