#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp`
CPPFLAGS="$CPPFLAGS -I../Include"
LDFLAGS=`pkg-config --libs glew assimp`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial21.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp camera.cpp ../Common/ogldev_basic_mesh.cpp lighting_technique.cpp ../Common/technique.cpp $CPPFLAGS $LDFLAGS -o tutorial21
