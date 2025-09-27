#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick freetype2 glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -std=c++20"
LDFLAGS=`pkg-config --libs glew ImageMagick freetype2 glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial14.cpp ../Common/ogldev_util.cpp ../Common/pipeline.cpp ../Common/math_3d.cpp ../Common/camera.cpp ../Common/ogldev_atb.cpp ../Common/glut_backend.cpp $CPPFLAGS $LDFLAGS -o tutorial14
