#!/bin/bash

CC=g++
CPPFLAGS="-std=c++20 -I../VulkanCore/Include -I../../Include -DVULKAN -ggdb3 -DOGLDEV_VULKAN"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS /usr/lib/x86_64-linux-gnu/libglslang.a /usr/lib/x86_64-linux-gnu/libglslang-default-resource-limits.a"

$CC tutorial17.cpp \
    ../VulkanCore/Source/core.cpp \
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
    $CPPFLAGS $LDFLAGS -o tutorial17
