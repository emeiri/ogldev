#!/bin/bash

CPPFLAGS="-I../../Include -ggdb3"
LDFLAGS=`pkg-config --libs assimp`

g++ assimp_report.cpp $CPPFLAGS $LDFLAGS -o assimp_report
