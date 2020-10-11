#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew`
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial01.cpp $CPPFLAGS $LDFLAGS -o tutorial01
