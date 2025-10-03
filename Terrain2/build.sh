#!/bin/bash

source ../build_base.sh

SOURCES="terrain_demo2.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp fault_formation_terrain.cpp $ROOTDIR/Common/ogldev_util.cpp $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/technique.cpp"

$CC $SOURCES $OGL_CPPFLAGS $OGL_LDFLAGS -o terrain_demo2
