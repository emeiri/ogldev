#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ freetype2 glfw3 fontconfig assimp`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -ggdb3 -DVULKAN"
LDFLAGS=`pkg-config --libs glew ImageMagick++ freetype2 glfw3 fontconfig assimp vulkan xcb`
LDFLAGS="$LDFLAGS -lglut ../Lib/libAntTweakBar.a -lX11  "

$CC tutorial52.cpp ../Common/ogldev_vulkan.cpp ../Common/ogldev_vulkan_core.cpp ../Common/ogldev_xcb_control.cpp ../Common/ogldev_basic_mesh.cpp ../Common/ogldev_basic_lighting.cpp ../Common/ogldev_util.cpp ../Common/pipeline.cpp ../Common/math_3d.cpp ../Common/camera.cpp ../Common/ogldev_atb.cpp ../Common/ogldev_backend.cpp ../Common/ogldev_glfw_backend.cpp ../Common/glut_backend.cpp ../Common/ogldev_texture.cpp  ../Common/technique.cpp ../Common/ogldev_app.cpp ../Common/FreetypeGL/freetypeGL.cpp $CPPFLAGS $LDFLAGS -o tutorial52
