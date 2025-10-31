#!/bin/bash

source ../build.sh

check_vulkan_sdk

CC=g++
CPPFLAGS="-I../../Common/3rdparty/ImGui/imGuIZMO.quat \
          -I../../Common/3rdparty/ImGui/GLFW \
	  -I../../Include/assimp5 \
	  -I../../Common \
	  -I../../DemoLITION/Framework/Include/ 
	  -I../VulkanCore/Include \
	  -I../../Include  \
	  -I$VULKAN_SDK/include \
          -DOGLDEV_VULKAN -ggdb3 -std=c++20 \
	  -DIMGUIZMO_MANUAL_IMGUI_INCLUDE"

 #-DIMGUIZMO_IMGUI_FOLDER=../../Common/3rdparty/ImGui/GLFW \

LDFLAGS=`pkg-config --libs glfw3 vulkan`

LDFLAGS="$LDFLAGS -L../../Lib -L$VULKAN_SDK/lib -lassimp -lglslang -lglslang-default-resource-limits -lSPIRV -lOGLCompiler -lOSDependent -lSPVRemapper -lspirv-cross-reflect -lmeshoptimizer"

$CC tutorial23.cpp \
    ../VulkanCore/Source/core.cpp \
    ../VulkanCore/Source/spirv_reflect.c \
    ../VulkanCore/Source/util.cpp \
    ../VulkanCore/Source/device.cpp \
    ../VulkanCore/Source/queue.cpp \
    ../VulkanCore/Source/wrapper.cpp \
    ../VulkanCore/Source/shader.cpp \
    ../VulkanCore/Source/model.cpp \
    ../VulkanCore/Source/texture.cpp \
    ../VulkanCore/Source/glfw_vulkan.cpp \
    ../VulkanCore/Source/ogldev_imgui.cpp \
    ../VulkanCore/Source/graphics_pipeline_v2.cpp \
    ../VulkanCore/Source/graphics_pipeline_v3.cpp \
    ../../Common/ogldev_util.cpp  \
    ../../Common/math_3d.cpp  \
    ../../Common/ogldev_glm_camera.cpp  \
    ../../Common/ogldev_glfw_camera_handler.cpp  \
  ../../Common/3rdparty/stb_image.cpp \
    ../../Common/ogldev_ect_cubemap.cpp \
    ../../DemoLITION/Framework/Source/core_model.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui_draw.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui_impl_glfw.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui_impl_vulkan.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui_tables.cpp \
    ../../Common/3rdparty/ImGui/GLFW/imgui_widgets.cpp \
    ../../Common/3rdparty/ImGui/imGuIZMO.quat/imguizmo_quat.cpp \
    $CPPFLAGS $LDFLAGS -o tutorial23
