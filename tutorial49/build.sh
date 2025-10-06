#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ freetype2 glfw3 fontconfig assimp`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -ggdb3 -std=c++20"
LDFLAGS=`pkg-config --libs glew ImageMagick++ freetype2 glfw3 fontconfig assimp`
LDFLAGS="$LDFLAGS -lglut -lX11 -lmeshoptimizer"

$CC tutorial49.cpp lighting_technique.cpp csm_technique.cpp ../Common/ogldev_framebuffer_object.cpp ../Common/ogldev_shadow_map_fbo.cpp  mesh.cpp ../Common/ogldev_basic_lighting.cpp ../Common/ogldev_util.cpp ../Common/pipeline.cpp ../Common/math_3d.cpp ../Common/camera.cpp ../Common/ogldev_atb.cpp ../Common/ogldev_backend.cpp ../Common/ogldev_glfw_backend.cpp ../Common/glut_backend.cpp ../Common/ogldev_texture.cpp  ../Common/technique.cpp ../Common/ogldev_app.cpp ../Common/FreetypeGL/freetypeGL.cpp ../Common/3rdparty/stb_image.cpp $CPPFLAGS $LDFLAGS -o tutorial49
