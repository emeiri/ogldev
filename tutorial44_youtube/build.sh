#!/bin/bash

OGLDEV_DIR=".."
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3 assimp`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3 assimp`
LDFLAGS="$LDFLAGS -lX11 -lmeshoptimizer"
SOURCES="tutorial44.cpp $OGLDEV_DIR/Common/ogldev_util.cpp $OGLDEV_DIR/Common/math_3d.cpp $OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp $OGLDEV_DIR/Common/ogldev_glfw.cpp $OGLDEV_DIR/Common/technique.cpp $OGLDEV_DIR/Common/ogldev_new_lighting.cpp $OGLDEV_DIR/Common/ogldev_basic_mesh.cpp $OGLDEV_DIR/Common/ogldev_texture.cpp $OGLDEV_DIR/Common/ogldev_world_transform.cpp $OGLDEV_DIR/Common/3rdparty/stb_image.cpp"

echo $SOURCES

$CC $SOURCES $CPPFLAGS $LDFLAGS -o tutorial44
