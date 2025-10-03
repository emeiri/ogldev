#!/bin/bash

source ../build_base.sh

SOURCES="terrain_demo3.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp midpoint_disp_terrain.cpp $ROOTDIR/Common/ogldev_util.cpp $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/technique.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp "

$CC $SOURCES $OGL_CPPFLAGS $OGL_LDFLAGS -o terrain_demo3
