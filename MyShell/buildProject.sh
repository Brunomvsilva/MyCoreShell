#!/bin/bash

set -e

rm -rf build
mkdir build
cd build

cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make -j$(nproc)

