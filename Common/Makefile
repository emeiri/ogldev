# libogldev_gl.so - the common OpenGL code

LIB := ogldev_gl
API := gl

IMGUI := 3rdparty/ImGui/GLFW

SRCS := $(wildcard *.cpp) \
        $(wildcard Techniques/*.cpp) \
        3rdparty/stb_image.cpp \
        $(IMGUI)/imgui.cpp \
        $(IMGUI)/imgui_draw.cpp \
        $(IMGUI)/imgui_tables.cpp \
        $(IMGUI)/imgui_widgets.cpp \
        $(IMGUI)/imgui_impl_glfw.cpp \
        $(IMGUI)/imgui_impl_opengl3.cpp

# Not part of the library, only linked as an object by the older tutorials
OBJ_ONLY_SRCS := FreetypeGL/freetypeGL.cpp

include ../Make/lib.mk
