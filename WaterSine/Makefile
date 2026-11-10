# WaterSine

TARGET := water_sine
API    := gl

SRCS := terrain_water.cpp \
        geomip_grid.cpp \
        terrain_technique.cpp \
        midpoint_disp_terrain.cpp \
        terrain.cpp \
        lod_manager.cpp \
        simple_water.cpp \
        simple_water_technique.cpp \
        triangle_list.cpp

# Common files, linked as ready-made objects
COMMON_OBJS := ogldev_util \
               math_3d \
               ogldev_basic_glfw_camera \
               ogldev_glfw \
               ogldev_stb_image \
               technique \
               ogldev_texture \
               cubemap_texture \
               ogldev_ect_cubemap \
               ogldev_skydome \
               ogldev_basic_mesh \
               ogldev_skydome_technique \
               ogldev_gui_texture \
               ogldev_guitex_technique \
               ogldev_screen_quad \
               ogldev_framebuffer \
               3rdparty/stb_image \
               3rdparty/ImGui/GLFW/imgui \
               3rdparty/ImGui/GLFW/imgui_draw \
               3rdparty/ImGui/GLFW/imgui_tables \
               3rdparty/ImGui/GLFW/imgui_widgets \
               3rdparty/ImGui/GLFW/imgui_impl_glfw \
               3rdparty/ImGui/GLFW/imgui_impl_opengl3

include ../Make/project.mk
