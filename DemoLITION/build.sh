#!/bin/bash

TARGET="../Lib/libdemolition.a"

if [[ ! -e "$TARGET" || $(($(date +%s) - $(stat -c %Y "$TARGET"))) -le 3600 ]]; then
	echo "No need to rebuild library"
	exit 0
fi

source ../build_base.sh

SOURCES="$ROOTDIR/Common/ogldev_util.cpp \
         $ROOTDIR/Common/math_3d.cpp \
	 $ROOTDIR/Common/ogldev_texture.cpp \
	 $ROOTDIR/Common/3rdparty/stb_image.cpp \
	 $ROOTDIR/Common/ogldev_glm_camera.cpp \
	 $ROOTDIR/Common/ogldev_glfw.cpp \
	 $ROOTDIR/Common/technique.cpp \
	 $ROOTDIR/Common/ogldev_framebuffer_object.cpp \
	 $ROOTDIR/Common/ogldev_framebuffer.cpp \
	 $ROOTDIR/Common/ogldev_shadow_cube_map_fbo.cpp \
	 $ROOTDIR/Common/ogldev_glfw_camera_handler.cpp \
	 $ROOTDIR/Common/cubemap_texture.cpp \
	 $ROOTDIR/Common/ogldev_ect_cubemap.cpp \
	 $ROOTDIR/Common/ogldev_shadow_mapping_technique_point_light.cpp \
	 Framework/Source/*.cpp \
	 Framework/Source/GL/*.cpp"
	 
IMGUI_SOURCES="$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
       	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp"

$CC $IMGUI_SOURCES $SOURCES $IMGUI_SOURCES $OGL_CPPFLAGS -IFramework/Include/ -I../Common -c

ar rcs $TARGET *.o
