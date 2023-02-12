#!/bin/bash

OGLDEV_DIR=".."
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3`
LDFLAGS="$LDFLAGS -lX11"
SOURCES="terrain_demo1.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp $OGLDEV_DIR/Common/ogldev_util.cpp $OGLDEV_DIR/Common/math_3d.cpp $OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp $OGLDEV_DIR/Common/ogldev_glfw.cpp $OGLDEV_DIR/Common/technique.cpp"

$CC $SOURCES $CPPFLAGS $LDFLAGS -o terrain_demo1
