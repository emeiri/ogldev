#!/bin/bash

CC=g++
CPPFLAGS="-std=c++20 -I../VulkanCore/Include -I../../Include -DVULKAN -ggdb3"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS /usr/lib/x86_64-linux-gnu/libglslang.a /usr/lib/x86_64-linux-gnu/libglslang-default-resource-limits.a"

$CC tutorial13.cpp \
    ../VulkanCore/Source/core.cpp \
    ../VulkanCore/Source/util.cpp \
    ../VulkanCore/Source/device.cpp \
    ../VulkanCore/Source/queue.cpp \
    ../VulkanCore/Source/wrapper.cpp \
    ../VulkanCore/Source/shader.cpp \
    ../../Common/ogldev_util.cpp  \
    ../../Common/3rdparty/stb_image.cpp \
    $CPPFLAGS $LDFLAGS -o tutorial13
