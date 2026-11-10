# Terrain4

TARGET := terrain_demo4
API    := gl

SRCS := terrain_demo4.cpp \
        single_tex_terrain_technique.cpp \
        texture_generator.cpp \
        terrain.cpp \
        triangle_list.cpp \
        terrain_technique.cpp \
        midpoint_disp_terrain.cpp

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
