#!/bin/bash

OGLDEV_DIR=".."
CC=g++
CPPFLAGS=`pkg-config --cflags glew glfw3 assimp`
CPPFLAGS="$CPPFLAGS -I$OGLDEV_DIR/Include -I$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW -ggdb3"
LDFLAGS=`pkg-config --libs glew glfw3 assimp`
LDFLAGS="$LDFLAGS -lX11 -ldl -lmeshoptimizer"
SOURCES="terrain_demo13.cpp \
	geomip_grid.cpp \
	terrain_technique.cpp \
	midpoint_disp_terrain.cpp \
	terrain.cpp \
	lod_manager.cpp \
	$OGLDEV_DIR/Common/ogldev_util.cpp \
	$OGLDEV_DIR/Common/math_3d.cpp \
	$OGLDEV_DIR/Common/ogldev_basic_glfw_camera.cpp \
	$OGLDEV_DIR/Common/ogldev_glfw.cpp \
	$OGLDEV_DIR/Common/ogldev_stb_image.cpp \
	$OGLDEV_DIR/Common/technique.cpp \
	$OGLDEV_DIR/Common/ogldev_texture.cpp \
	$OGLDEV_DIR/Common/cubemap_texture.cpp \
	$OGLDEV_DIR/Common/ogldev_skydome.cpp \
	$OGLDEV_DIR/Common/ogldev_basic_mesh.cpp \
	$OGLDEV_DIR/Common/ogldev_skydome_technique.cpp \
	$OGLDEV_DIR/Common/3rdparty/stb_image.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
	$OGLDEV_DIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp "

$CC $SOURCES $CPPFLAGS $LDFLAGS -o terrain_demo13
