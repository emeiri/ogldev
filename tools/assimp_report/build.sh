#!/bin/bash

CPPFLAGS="-I../../Include -ggdb3"
LDFLAGS="" #`pkg-config --libs assimp`

g++ assimp_report.cpp -I/usr/local/include $CPPFLAGS -L/usr/local/lib -lassimp $LDFLAGS -o assimp_report
