#!/bin/bash

CC=g++
CPPFLAGS="-I../VulkanCore/Include -I../../Include -DVULKAN -ggdb3"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS"

$CC tutorial04.cpp ../VulkanCore/Source/core.cpp ../VulkanCore/Source/util.cpp ../VulkanCore/Source/device.cpp ../../Common/ogldev_util.cpp  $CPPFLAGS $LDFLAGS -o tutorial04
