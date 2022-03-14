#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -I/usr/include/freetype2 -ggdb3"
LDFLAGS=`pkg-config --libs glew assimp`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC glfw_debug_output.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp camera.cpp skinning_technique.cpp skinned_mesh.cpp ../Common/technique.cpp $CPPFLAGS $LDFLAGS -o glfw_debug_output
