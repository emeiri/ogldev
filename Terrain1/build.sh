#!/bin/bash

source ../build_base.sh

SOURCES="terrain_demo1.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp $ROOTDIR/Common/ogldev_util.cpp $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/technique.cpp"

$CC $SOURCES $OGL_CPPFLAGS $OGL_LDFLAGS -o terrain_demo1
