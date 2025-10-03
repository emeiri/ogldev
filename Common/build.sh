#!/bin/bash

TARGET="../Lib/libogldev.a"

if [[ -e "$TARGET" && $(($(date +%s) - $(stat -c %Y "$TARGET"))) -le 3600 ]]; then
	echo "No need to rebuild library"
	exit 0
fi

source ../build_base.sh

IMGUI_SOURCES="$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
       	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
	       $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp"

$CC $OGL_CPPFLAGS -c *.cpp Techniques/*.cpp 3rdparty/stb_image.cpp $IMGUI_SOURCES

ar rcs $TARGET *.o
