#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew assimp glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common -I../DemoLITION/Framework/Include -I../DemoLITION/Framework/Include/Int -I../DemoLITION/Framework/Include/GL -ggdb3 -I../Common/3rdparty/ImGui/GLFW/"
LDFLAGS=`pkg-config --libs glew assimp glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11 -lmeshoptimizer"
ROOTDIR=".."

IMGUI_SOURCES="$ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_draw.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_impl_opengl3.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_tables.cpp $ROOTDIR/Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp"

$CC tutorial55.cpp $ROOTDIR/Common/ogldev_util.cpp  $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_texture.cpp $ROOTDIR/Common/3rdparty/stb_image.cpp $ROOTDIR/Common/ogldev_world_transform.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_phong_renderer.cpp  $ROOTDIR/Common/ogldev_basic_mesh.cpp $ROOTDIR/Common/ogldev_skinned_mesh.cpp $ROOTDIR/Common/ogldev_skinning_technique.cpp $ROOTDIR/Common/ogldev_new_lighting.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/technique.cpp $ROOTDIR/Common/ogldev_shadow_mapping_technique.cpp $ROOTDIR/Common/ogldev_base_app.cpp $ROOTDIR/Common/ogldev_quad_tess_technique.cpp ../DemoLITION/Framework/Source/*.cpp ../DemoLITION/Framework/Source/GL/*.cpp ../Common/ogldev_shadow_map_fbo.cpp ../Common/ogldev_shadow_cube_map_fbo.cpp ../Common/ogldev_shadow_mapping_technique_point_light.cpp $ROOTDIR/Common/ogldev_glm_camera.cpp $ROOTDIR/Common/ogldev_glfw_camera_handler.cpp $IMGUI_SOURCES $CPPFLAGS $LDFLAGS -o tutorial55
