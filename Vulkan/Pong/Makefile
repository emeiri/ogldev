# Vulkan/Pong

TARGET := vulkan_pong
API    := vulkan

# The game logic is shared with the other Pong versions in Games/Pong
SRCS := $(wildcard *.cpp) \
        ../../Games/Pong/pong.cpp

LIBS := vulkancore

EXTRA_CPPFLAGS := -I../../Games/Pong

include ../../Make/project.mk
