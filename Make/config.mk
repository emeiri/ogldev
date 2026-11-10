# ---------------------------------------------------------------------------
#  Global build configuration, included by every Makefile in the tree.
#  Edit compiler flags and system libraries here.
# ---------------------------------------------------------------------------

ifndef OGLDEV_CONFIG_MK
OGLDEV_CONFIG_MK := 1

# make splits paths on spaces, so it can't work in such a directory
ifneq ($(words $(CURDIR)),1)
  $(error The path "$(CURDIR)" contains a space. Move the repository to a path without spaces)
endif

# Root of the repository (the parent of this Make/ directory)
ROOT   := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
LIBDIR := $(ROOT)/Lib/linux

SHELL := /bin/bash

# Use all cores by default. Only the top level make adds -j, the jobserver
# then shares the job slots with all the sub-makes. 'make -jN' overrides it.
NPROC := $(shell nproc)
ifeq ($(MAKELEVEL),0)
  ifeq ($(filter -j% j%,$(MAKEFLAGS)),)
    MAKEFLAGS += -j$(NPROC)
  endif
  MAKEFLAGS += --output-sync=target --no-print-directory
endif

CXX := g++

# Flags shared by every compilation unit.
# -MMD -MP generate the header dependency files (*.d)
BASE_CXXFLAGS := -std=c++20 -ggdb3 -fPIC -MMD -MP

# ---------------------------------------------------------------------------
#  OpenGL
# ---------------------------------------------------------------------------
GL_PKGS     := glew glfw3 assimp freetype2 fontconfig ImageMagick++
GL_CPPFLAGS := $(shell pkg-config --cflags $(GL_PKGS)) \
               -I$(ROOT)/Include \
               -I$(ROOT)/Common/FreetypeGL \
               -I$(ROOT)/Common/3rdparty/ImGui/GLFW
GL_LDLIBS   := $(shell pkg-config --libs $(GL_PKGS)) -lglut -lX11 -lmeshoptimizer -ldl

# ---------------------------------------------------------------------------
#  Vulkan (requires the LunarG SDK: source <sdk dir>/setup-env.sh)
# ---------------------------------------------------------------------------
VK_CPPFLAGS := -DOGLDEV_VULKAN -DIMGUIZMO_MANUAL_IMGUI_INCLUDE \
               -I$(ROOT)/Common/3rdparty/ImGui/imGuIZMO.quat \
               -I$(ROOT)/Common/3rdparty/ImGui/GLFW \
               -I$(ROOT)/Include/assimp5 \
               -I$(ROOT)/Common \
               -I$(ROOT)/DemoLITION/Framework/Include \
               -I$(ROOT)/Vulkan/VulkanCore/Include \
               -I$(ROOT)/Include \
               -I$(VULKAN_SDK)/include
VK_LDLIBS   := $(shell pkg-config --libs glfw3 vulkan 2>/dev/null) -L$(VULKAN_SDK)/lib \
               -lassimp -lglslang -lglslang-default-resource-limits -lSPIRV \
               -lOSDependent -lSPVRemapper -lspirv-cross-reflect -lmeshoptimizer

VK_SDK_MISSING := Missing environment variable VULKAN_SDK. Download the SDK from \
  https://vulkan.lunarg.com/sdk/home\#linux, extract it and run: source <extracted dir>/setup-env.sh

# ---------------------------------------------------------------------------
#  Shared libraries: name -> directory that builds it, and what it needs
# ---------------------------------------------------------------------------
LIB_DIR_ogldev_gl     := $(ROOT)/Common
LIB_DIR_demolition_gl := $(ROOT)/DemoLITION
LIB_DIR_vulkancore    := $(ROOT)/Vulkan/VulkanCore

# Linking a library also needs the libraries it depends on
LIB_DEPS_demolition_gl := ogldev_gl

# Extra include directories for projects that use a library
LIB_CPPFLAGS_demolition_gl := -I$(ROOT)/DemoLITION/Framework/Include

# Executables find the shared libraries in Lib/linux at runtime through a
# path relative to their own location, so the tree can be moved
RPATH_FLAGS = -Wl,-rpath,'$$ORIGIN/$(shell realpath -m --relative-to=$(CURDIR) $(LIBDIR))'

# Objects of the current directory go to its build/ subdirectory
OBJDIR ?= $(CURDIR)/build

# Compile rules. Objects are named after the full source path relative to
# ROOT so that files from different directories never collide.
$(OBJDIR)/%.cpp.o: $(ROOT)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(BASE_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.c.o: $(ROOT)/%.c
	@mkdir -p $(@D)
	$(CXX) $(BASE_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Convert source paths (relative to the current directory) to object paths
src_to_obj = $(patsubst $(ROOT)/%,$(OBJDIR)/%.o,$(abspath $(1)))

endif
