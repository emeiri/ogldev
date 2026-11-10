# libvulkancore.so - the Vulkan framework, together with the parts of Common
# and DemoLITION it uses (all compiled with -DOGLDEV_VULKAN)

LIB := vulkancore
API := vulkan

COMMON := ../../Common
IMGUI  := $(COMMON)/3rdparty/ImGui/GLFW

SRCS := $(wildcard Source/*.cpp) \
        $(wildcard Source/Pipelines/*.cpp) \
        Source/spirv_reflect.c \
        $(COMMON)/ogldev_util.cpp \
        $(COMMON)/math_3d.cpp \
        $(COMMON)/ogldev_glm_camera.cpp \
        $(COMMON)/ogldev_glfw_camera_handler.cpp \
        $(COMMON)/ogldev_ect_cubemap.cpp \
        $(COMMON)/3rdparty/stb_image.cpp \
        ../../DemoLITION/Framework/Source/core_model.cpp \
        ../../DemoLITION/Framework/Source/assimp_model_loader.cpp \
        $(IMGUI)/imgui.cpp \
        $(IMGUI)/imgui_draw.cpp \
        $(IMGUI)/imgui_tables.cpp \
        $(IMGUI)/imgui_widgets.cpp \
        $(IMGUI)/imgui_impl_glfw.cpp \
        $(IMGUI)/imgui_impl_vulkan.cpp \
        $(COMMON)/3rdparty/ImGui/imGuIZMO.quat/imguizmo_quat.cpp

include ../../Make/lib.mk
