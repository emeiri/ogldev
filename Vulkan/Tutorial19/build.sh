#!/bin/bash

source ../build.sh

check_vulkan_sdk

CC=g++
CPPFLAGS="-I../VulkanCore/Include -I../../Include  -I$VULKAN_SDK/include -DOGLDEV_VULKAN -ggdb3 -std=c++20"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS -L$VULKAN_SDK/lib -lglslang -lglslang-default-resource-limits -lSPIRV -lOGLCompiler -lOSDependent -lSPVRemapper \
	-lspirv-cross-reflect"
$CC tutorial19.cpp \
    ../VulkanCore/Source/core.cpp \
    ../VulkanCore/Source/spirv_reflect.c \
    ../VulkanCore/Source/util.cpp \
    ../VulkanCore/Source/device.cpp \
    ../VulkanCore/Source/queue.cpp \
    ../VulkanCore/Source/wrapper.cpp \
    ../VulkanCore/Source/shader.cpp \
    ../VulkanCore/Source/glfw_vulkan.cpp \
    ../VulkanCore/Source/graphics_pipeline.cpp \
    ../../Common/ogldev_util.cpp  \
    ../../Common/ogldev_glm_camera.cpp  \
    ../../Common/ogldev_glfw_camera_handler.cpp  \
  ../../Common/3rdparty/stb_image.cpp \
    ../../Common/ogldev_ect_cubemap.cpp \
    $CPPFLAGS $LDFLAGS -o tutorial19
