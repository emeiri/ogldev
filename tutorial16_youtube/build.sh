#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++`
CPPFLAGS="$CPPFLAGS -I../Include"
LDFLAGS=`pkg-config --libs glew ImageMagick++`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial16.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp world_transform.cpp camera.cpp $CPPFLAGS $LDFLAGS -o tutorial16
