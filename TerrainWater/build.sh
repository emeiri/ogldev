#!/bin/bash

source ../build_base.sh

SOURCES="terrain_water.cpp \
	geomip_grid.cpp \
	terrain_technique.cpp \
	midpoint_disp_terrain.cpp \
	terrain.cpp \
	lod_manager.cpp \
    simple_water.cpp \
    simple_water_technique.cpp
    triangle_list.cpp \
	$ROOTDIR/Common/ogldev_util.cpp \
	$ROOTDIR/Common/math_3d.cpp \
	$ROOTDIR/Common/ogldev_basic_glfw_camera.cpp \
	$ROOTDIR/Common/ogldev_glfw.cpp \
	$ROOTDIR/Common/ogldev_stb_image.cpp \
	$ROOTDIR/Common/technique.cpp \
	$ROOTDIR/Common/ogldev_texture.cpp \
	$ROOTDIR/Common/cubemap_texture.cpp \
	$ROOTDIR/Common/ogldev_ect_cubemap.cpp \
	$ROOTDIR/Common/ogldev_skydome.cpp \
	$ROOTDIR/Common/ogldev_basic_mesh.cpp \
	$ROOTDIR/Common/ogldev_skydome_technique.cpp \
	$ROOTDIR/Common/ogldev_gui_texture.cpp \
	$ROOTDIR/Common/ogldev_guitex_technique.cpp \
	$ROOTDIR/Common/ogldev_screen_quad.cpp \
	$ROOTDIR/Common/ogldev_framebuffer.cpp \
	$ROOTDIR/Common/3rdparty/stb_image.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
	$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp "

$CC $SOURCES $OGL_CPPFLAGS $OGL_LDFLAGS -o terrain_water
