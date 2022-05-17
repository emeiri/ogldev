#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew assimp glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -ggdb3"
LDFLAGS=`pkg-config --libs glew assimp glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11"
ROOTDIR=".."

$CC tutorial35.cpp $ROOTDIR/Common/ogldev_util.cpp  $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_texture.cpp $ROOTDIR/Common/3rdparty/stb_image.cpp $ROOTDIR/Common/ogldev_world_transform.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_phong_renderer.cpp  $ROOTDIR/Common/ogldev_basic_mesh.cpp $ROOTDIR/Common/ogldev_skinned_mesh.cpp $ROOTDIR/Common/ogldev_skinning_technique.cpp $ROOTDIR/Common/ogldev_new_lighting.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/ogldev_shadow_mapping_technique.cpp $ROOTDIR/Common/ogldev_shadow_map_fbo.cpp $ROOTDIR/Common/technique.cpp $CPPFLAGS $LDFLAGS -o tutorial35
