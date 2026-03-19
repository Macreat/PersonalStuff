#!/bin/bash
set -e
pushd c_project
make
if [ ! -f iqproc ]; then echo "Build failed"; exit 2; fi
mkdir -p results
/usr/bin/time -v ./iqproc "$@" 2> results/last_c_time.txt | tee results/last_c_out.txt
popd
echo "C run complete; results in c_project/results/"