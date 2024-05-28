#!/bin/bash
# export CC=/usr/bin/gcc-9
# export CXX=/usr/bin/g++-9
# if build & output & logdir directory does not exist, create it
if [ ! -d "build" ]; then
  mkdir build
fi
if [ ! -d "output" ]; then
  mkdir output
fi
if [ ! -d "logdir" ]; then
  mkdir logdir
fi
if [ ! -d "benchmarks" ]; then
  mkdir benchmarks
fi
cd ./build
rm -rf *
cmake ..
make -j16
cd ..