#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ freetype2 glfw3 fontconfig assimp`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -ggdb3 -DOGLDEV_VULKAN -std=c++20"
LDFLAGS=`pkg-config --libs glew ImageMagick++ freetype2 glfw3 fontconfig assimp vulkan xcb`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial54.cpp ../Common/ogldev_vulkan.cpp ../Common/ogldev_vulkan_core.cpp ../Common/ogldev_xcb_control.cpp ../Common/math_3d.cpp ../Common/ogldev_util.cpp $CPPFLAGS $LDFLAGS -o tutorial54
