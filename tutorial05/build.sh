#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick freetype2 glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL"
LDFLAGS=`pkg-config --libs glew ImageMagick freetype2 glfw3`
LDFLAGS="$LDFLAGS -lglut"

$CC tutorial05.cpp ../Common/ogldev_util.cpp $CPPFLAGS $LDFLAGS -o tutorial05
