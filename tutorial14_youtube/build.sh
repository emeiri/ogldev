#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick freetype2 glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL"
LDFLAGS=`pkg-config --libs glew ImageMagick freetype2 glfw3`
LDFLAGS="$LDFLAGS -lglut ../Lib/libAntTweakBar.a -lX11"

$CC tutorial14.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp world_transform.cpp camera.cpp $CPPFLAGS $LDFLAGS -o tutorial14
