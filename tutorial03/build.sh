#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew`
CPPFLAGS="$CPPFLAGS -I../Include"
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial03.cpp $CPPFLAGS $LDFLAGS -o tutorial03
