#!/bin/bash

OGLDEV_DIR=".."
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -I$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3`
LDFLAGS="$LDFLAGS -lX11 -ldl"
SOURCES="terrain_demo3.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp midpoint_disp_terrain.cpp $OGLDEV_DIR/Common/ogldev_util.cpp $OGLDEV_DIR/Common/math_3d.cpp $OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp $OGLDEV_DIR/Common/ogldev_glfw.cpp $OGLDEV_DIR/Common/technique.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp $OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp "

#SOURCES="terrain_demo3.cpp terrain.cpp triangle_list.cpp terrain_technique.cpp midpoint_displacement_terrain.cpp $OGLDEV_DIR/Common/ogldev_util.cpp $OGLDEV_DIR/Common/math_3d.cpp $OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp $OGLDEV_DIR/Common/ogldev_glfw.cpp $OGLDEV_DIR/Common/technique.cpp"

$CC $SOURCES $CPPFLAGS $LDFLAGS -o terrain_demo3
