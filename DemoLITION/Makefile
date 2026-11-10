# libdemolition_gl.so - the DemoLITION framework (OpenGL)

LIB := demolition_gl
API := gl

SRCS := $(wildcard Framework/Source/*.cpp) \
        $(wildcard Framework/Source/GL/*.cpp) \
        $(wildcard Framework/Source/Services/*.cpp) \
        ../Common/3rdparty/ImGui/imGuIZMO.quat/imguizmo_quat.cpp   # includes imGuIZMOquat.cpp

EXTRA_CPPFLAGS := -IFramework/Include -I../Common -DIMGUIZMO_IMGUI_FOLDER="3rdparty/ImGui/GLFW/"

include ../Make/lib.mk
