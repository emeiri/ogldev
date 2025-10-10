#!/bin/bash

source ../build.sh

check_vulkan_sdk

CC=g++
CPPFLAGS="-I../VulkanCore/Include -I../../Include -DVULKAN -ggdb3 -std=c++20"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS"

$CC tutorial11.cpp \
    ../VulkanCore/Source/core.cpp \
    ../VulkanCore/Source/util.cpp \
    ../VulkanCore/Source/device.cpp \
    ../VulkanCore/Source/queue.cpp \
    ../VulkanCore/Source/wrapper.cpp \
    ../../Common/ogldev_util.cpp  \
    ../../Common/3rdparty/stb_image.cpp \
    ../../Common/ogldev_ect_cubemap.cpp \
    $CPPFLAGS $LDFLAGS -o tutorial11
