# Terrain6

TARGET := terrain_demo6
API    := gl

SRCS := terrain_demo6.cpp \
        geomip_grid.cpp \
        terrain_technique.cpp \
        midpoint_disp_terrain.cpp \
        terrain.cpp

# Common files, linked as ready-made objects
COMMON_OBJS := ogldev_util \
               math_3d \
               ogldev_basic_glfw_camera \
               ogldev_glfw \
               ogldev_stb_image \
               technique \
               ogldev_texture \
               3rdparty/stb_image \
               3rdparty/ImGui/GLFW/imgui \
               3rdparty/ImGui/GLFW/imgui_draw \
               3rdparty/ImGui/GLFW/imgui_tables \
               3rdparty/ImGui/GLFW/imgui_widgets \
               3rdparty/ImGui/GLFW/imgui_impl_glfw \
               3rdparty/ImGui/GLFW/imgui_impl_opengl3

include ../Make/project.mk
