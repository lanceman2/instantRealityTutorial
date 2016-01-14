#!/bin/bash

scriptdir="$(dirname ${BASH_SOURCE[0]})" || exit 1
cd "$scriptdir" || exit 1
./build.bash || exit 1

dtk-floatSliders head\
 -N 6\
 -s 0 -1.2 1.2 0\
 -s 1 -1.2 1.2 0\
 -s 2 -1.2 1.2 0\
 -s 3 -180 180 0\
 -s 4 -180 180 0\
 -s 5 -180 180 0\
 -l x y z H P R &

[ "$?" = "0" ] || exit 1

InstantPlayer teapot.x3d
