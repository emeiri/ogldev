#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb` 
CPPFLAGS="-c $CPPFLAGS -DVULKAN -I../Include -I../Include/ImageMagick-6/ -I../Common/FreetypeGL -I/usr/include/freetype2"
LDFLAGS=`pkg-config --libs glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb`
LDFLAGS="$LDFLAGS -lglut -lxcb -lvulkan    ../Lib/libAntTweakBar.a -lX11"

PWD=`pwd`
PROJ_NAME=`basename $PWD`
PROJ_SRC=$PROJ_NAME.cpp
PROJ_OBJ=$PROJ_NAME.o

$CC $CPPFLAGS $PROJ_SRC -o $PROJ_OBJ
$CC $PROJ_OBJ -o $PROJ_NAME $LDFLAGS
