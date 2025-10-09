#!/bin/bash

source ../build.sh

check_vulkan_sdk

CC=g++
CPPFLAGS="-I../../Include/assimp5 -I../../Common -I../../DemoLITION/Framework/Include/ -I../VulkanCore/Include -I../../Include  -I$VULKAN_SDK/include -DOGLDEV_VULKAN -ggdb3 -std=c++20"
LDFLAGS=`pkg-config --libs glfw3 vulkan`
LDFLAGS="$LDFLAGS -L../../Lib -L../../Lib -L$VULKAN_SDK/lib -lassimp -lglslang -lglslang-default-resource-limits -lSPIRV -lOGLCompiler -lOSDependent -lSPVRemapper -lspirv-cross-reflect ../../Lib/libmeshoptimizer.a"
$CC tutorial20.cpp \
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
    ../VulkanCore/Source/graphics_pipeline_v2.cpp \
    ../../Common/ogldev_util.cpp  \
    ../../Common/math_3d.cpp  \
    ../../Common/ogldev_glm_camera.cpp  \
    ../../Common/ogldev_glfw_camera_handler.cpp  \
  ../../Common/3rdparty/stb_image.cpp \
    ../../Common/ogldev_ect_cubemap.cpp \
    ../../DemoLITION/Framework/Source/core_model.cpp \
    $CPPFLAGS $LDFLAGS -o tutorial20
