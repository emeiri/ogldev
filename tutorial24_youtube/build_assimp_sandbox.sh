#!/bin/bash

CPPFLAGS="-ggdb3"
LDFLAGS=`pkg-config --libs assimp`

g++ assimp_sandbox.cpp $CPPFLAGS $LDFLAGS -o assimp_sandbox
