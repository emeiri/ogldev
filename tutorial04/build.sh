#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick freetype2`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL"
LDFLAGS=`pkg-config --libs glew ImageMagick freetype2`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial04.cpp ../Common/ogldev_util.cpp $CPPFLAGS $LDFLAGS -o tutorial04
