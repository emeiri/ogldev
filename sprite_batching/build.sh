#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp`
CPPFLAGS="$CPPFLAGS -I../Include"
LDFLAGS=`pkg-config --libs glew assimp`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC sprite_batch.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp camera.cpp ../Common/ogldev_basic_mesh.cpp $CPPFLAGS $LDFLAGS -o sprite_batch
