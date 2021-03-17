#!/bin/bash

CC=g++
CPPFLAGS="-I../Include"
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial02.cpp $CPPFLAGS $LDFLAGS -o tutorial02
