# Terrain3

TARGET := terrain_demo3
API    := gl

SRCS := terrain_demo3.cpp \
        terrain.cpp \
        triangle_list.cpp \
        terrain_technique.cpp \
        midpoint_disp_terrain.cpp

# Common files, linked as ready-made objects
COMMON_OBJS := ogldev_util \
               math_3d \
               ogldev_basic_glfw_camera \
               ogldev_glfw \
               technique \
               3rdparty/ImGui/GLFW/imgui \
               3rdparty/ImGui/GLFW/imgui_draw \
               3rdparty/ImGui/GLFW/imgui_tables \
               3rdparty/ImGui/GLFW/imgui_widgets \
               3rdparty/ImGui/GLFW/imgui_impl_glfw \
               3rdparty/ImGui/GLFW/imgui_impl_opengl3

include ../Make/project.mk
