# ---------------------------------------------------------------------------
#  Rules for building a project (an executable).
#
#  The project Makefile sets:
#    TARGET         name of the executable
#    API            gl or vulkan, selects the compiler flags and system libraries
#    SRCS           the project's own sources
#    LIBS           (optional) shared libraries to link: ogldev_gl demolition_gl vulkancore
#    COMMON_OBJS    (optional) Common files to link as ready-made objects instead
#                   of linking the whole library, e.g. ogldev_util math_3d
#                   (the objects are built together with libogldev_gl)
#    EXTRA_CPPFLAGS (optional)
#    EXTRA_LDLIBS   (optional)
#
#  make          build, output to the screen and to build.log
#  make clean    remove the objects and the executable
#
#  The libraries are built first. Pass NO_LIB_BUILD=1 to skip that step
#  (the master build does it once for all projects).
# ---------------------------------------------------------------------------

include $(dir $(lastword $(MAKEFILE_LIST)))config.mk

ifeq ($(API),vulkan)
  ifeq ($(VULKAN_SDK),)
    $(error $(VK_SDK_MISSING))
  endif
  CPPFLAGS := $(VK_CPPFLAGS) $(EXTRA_CPPFLAGS)
  LDLIBS   := $(VK_LDLIBS) $(EXTRA_LDLIBS)
else
  CPPFLAGS := $(GL_CPPFLAGS) $(EXTRA_CPPFLAGS)
  LDLIBS   := $(GL_LDLIBS) $(EXTRA_LDLIBS)
endif

# Add the libraries that the requested libraries depend on
ifneq ($(COMMON_OBJS),)
  LIBS_TO_BUILD := ogldev_gl
endif
LIBS_TO_BUILD += $(foreach l,$(LIBS),$(LIB_DEPS_$(l)) $(l))
LIBS_TO_BUILD := $(sort $(LIBS_TO_BUILD))
LINK_LIBS     := $(LIBS) $(filter-out $(LIBS),$(foreach l,$(LIBS),$(LIB_DEPS_$(l))))

CPPFLAGS += $(foreach l,$(LIBS),$(LIB_CPPFLAGS_$(l)))

OBJS             := $(call src_to_obj,$(SRCS))
LIB_FILES        := $(foreach l,$(LIBS_TO_BUILD),$(LIBDIR)/lib$(l).so)
COMMON_OBJ_FILES := $(foreach f,$(COMMON_OBJS),$(firstword $(wildcard $(ROOT)/Common/$(f).cpp $(ROOT)/Common/$(f).c) $(ROOT)/Common/$(f).cpp))
COMMON_OBJ_FILES := $(patsubst $(ROOT)/%,$(LIB_DIR_ogldev_gl)/build/%.o,$(COMMON_OBJ_FILES))

.PHONY: all build clean FORCE
.DEFAULT_GOAL := all

# Build and save the output to build.log
all:
	+@set -o pipefail; $(MAKE) build 2>&1 | tee build.log

build: $(TARGET)

$(TARGET): $(OBJS) $(COMMON_OBJ_FILES) $(LIB_FILES)
	$(CXX) -o $@ $(OBJS) $(COMMON_OBJ_FILES) -L$(LIBDIR) $(addprefix -l,$(LINK_LIBS)) \
		$(RPATH_FLAGS) $(LDLIBS)

# The Common objects are made by the libogldev_gl build
$(COMMON_OBJ_FILES): $(LIBDIR)/libogldev_gl.so ;

ifndef NO_LIB_BUILD
# Run the library's own make every time. It does nothing when the library is up to date.
# A library's dependencies are built first, so each library directory is entered once.
$(LIB_FILES): FORCE
	+@$(MAKE) -C $(LIB_DIR_$(patsubst lib%.so,%,$(notdir $@))) NO_LIB_BUILD=1
$(foreach l,$(LIBS_TO_BUILD),$(foreach d,$(LIB_DEPS_$(l)),$(eval $(LIBDIR)/lib$(l).so: $(LIBDIR)/lib$(d).so)))
endif

FORCE:

clean:
	rm -rf $(OBJDIR) $(TARGET) build.log

-include $(OBJS:.o=.d)
