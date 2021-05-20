#!/bin/bash

CC=g++
CPPFLAGS="-I../Include"
LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial10.cpp ../Common/ogldev_util.cpp $CPPFLAGS $LDFLAGS -o tutorial10
