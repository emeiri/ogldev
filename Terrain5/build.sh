#!/bin/bash

OGLDEV_DIR="/home/emeiri/projects/ogldev"
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3`
LDFLAGS="$LDFLAGS -lX11"
SOURCES="terrain_demo5.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp midpoint_displacement_terrain.cpp $OGLDEV_DIR/Common/ogldev_util.cpp $OGLDEV_DIR/Common/math_3d.cpp $OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp $OGLDEV_DIR/Common/ogldev_glfw.cpp $OGLDEV_DIR/Common/technique.cpp"

echo $SOURCES

$CC $SOURCES $CPPFLAGS $LDFLAGS -o demo5
