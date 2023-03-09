#!/bin/bash

OGLDEV_DIR=".."
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -I$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3`
LDFLAGS="$LDFLAGS -lX11"
SOURCES="terrain_demo5.1.cpp \
	triangle_list.cpp \
	terrain_technique.cpp \
	midpoint_disp_terrain.cpp \
	terrain.cpp \
	slope_lighter.cpp \
	$OGLDEV_DIR/Common/ogldev_util.cpp \
	$OGLDEV_DIR/Common/math_3d.cpp \
	$OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp \
	$OGLDEV_DIR/Common/ogldev_glfw.cpp \
	$OGLDEV_DIR/Common/ogldev_stb_image.cpp \
	$OGLDEV_DIR/Common/technique.cpp \
	$OGLDEV_DIR/Common/ogldev_texture.cpp \
	$OGLDEV_DIR/Common/3rdparty/stb_image.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp "

$CC $SOURCES $CPPFLAGS $LDFLAGS -o terrain_demo5.1

