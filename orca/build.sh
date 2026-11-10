#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp`
CPPFLAGS="$CPPFLAGS -I../Include -ggdb3 -std=c++20"
LDFLAGS=`pkg-config --libs glew ImageMagick++ assimp glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11 -lmeshoptimizer"

$CC tutorial23.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp camera.cpp ../Common/ogldev_basic_mesh.cpp lighting_technique.cpp ../Common/technique.cpp ../Common/ogldev_glfw.cpp $CPPFLAGS $LDFLAGS -o tutorial23
