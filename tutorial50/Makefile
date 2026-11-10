# tutorial50

TARGET := tutorial50
API    := gl

SRCS := tutorial50.cpp \
        ../Common/ogldev_vulkan.cpp \
        ../Common/ogldev_vulkan_core.cpp \
        ../Common/ogldev_xcb_control.cpp \
        ../Common/math_3d.cpp \
        ../Common/ogldev_util.cpp

EXTRA_CPPFLAGS := -DOGLDEV_VULKAN

EXTRA_LDLIBS := $(shell pkg-config --libs vulkan xcb)

include ../Make/project.mk
