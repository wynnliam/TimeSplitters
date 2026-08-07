#!/bin/bash

files="./src/main.c \
./src/ts/packfile/packfile.c \
./src/ts/logo/logo.c"

echo BASE
gcc -Iinc/ $files -g -o TimeSplitters.out

echo TSPACK
gcc -Iinc/ ./tools/tspack.c ./src/ts/packfile/packfile.c -g -o tspack.out
