#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -ggdb -O0"
LDFLAGS=`pkg-config --libs glew assimp glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial33.cpp quad_array.cpp sprite_batch.cpp ../Common/ogldev_tex_technique.cpp ../Common/ogldev_sprite_technique.cpp ../Common/ogldev_new_lighting.cpp ../Common/ogldev_glfw.cpp ../Common/technique.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp ../Common/ogldev_basic_glfw_camera.cpp ../Common/ogldev_basic_mesh.cpp $CPPFLAGS $LDFLAGS -o tutorial33
