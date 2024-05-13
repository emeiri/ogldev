#!/bin/bash

CC=g++
CPPFLAGS="-I../Include -ggdb3"
LDFLAGS=`pkg-config --libs glfw3`
LDFLAGS="$LDFLAGS"

$CC tutorial01.cpp $CPPFLAGS $LDFLAGS -o tutorial01
