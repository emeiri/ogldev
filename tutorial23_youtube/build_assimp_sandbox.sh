#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags assimp`
CPPFLAGS="$CPPFLAGS -I../Include -ggdb3"
LDFLAGS=`pkg-config --libs assimp`


$CC assimp_sandbox.cpp  $CPPFLAGS $LDFLAGS -o assimp_sandbox
