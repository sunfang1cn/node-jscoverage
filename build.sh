#!/bin/sh
mv ./Makefile.in ./Makefile.bin
mv ./Makefile.node ./Makefile.in
./configure
make
g++  -DHAVE_CONFIG_H -I.    -Ijs -Ijs/obj -DXP_UNIX -g -O2 -I ${NODE_INCLUDE:='/usr/local/include/node'} -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wall -fPIC -MT jscoverage.o -MD -MP -MF .deps/jscoverage.Tpo -c -o jscoverage.o jscoverage.cpp
make
mv ./Makefile.bin ./Makefile.in