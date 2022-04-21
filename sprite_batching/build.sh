#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp glfw3`
CPPFLAGS="$CPPFLAGS -I../Include"
LDFLAGS=`pkg-config --libs glew assimp glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC sprite_batch.cpp quad_array.cpp ../Common/ogldev_flat_passthru_technique.cpp ../Common/ogldev_new_lighting.cpp ../Common/ogldev_glfw.cpp ../Common/technique.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp ../Common/ogldev_basic_glfw_camera.cpp ../Common/ogldev_basic_mesh.cpp $CPPFLAGS $LDFLAGS -o sprite_batch
