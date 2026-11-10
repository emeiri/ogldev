# ---------------------------------------------------------------------------
#  Rules for building one of the shared libraries in Lib/linux.
#
#  The including Makefile sets:
#    LIB            library name, builds Lib/linux/lib$(LIB).so
#    API            gl or vulkan, selects the compiler flags and system libraries
#    SRCS           sources that go into the library
#    OBJ_ONLY_SRCS  (optional) sources compiled to objects that projects can link
#                   directly but which are not part of the library
#    EXTRA_CPPFLAGS (optional)
#
#  Pass NO_LIB_BUILD=1 to skip building the libraries this one depends on.
# ---------------------------------------------------------------------------

include $(dir $(lastword $(MAKEFILE_LIST)))config.mk

ifeq ($(API),vulkan)
  ifeq ($(VULKAN_SDK),)
    $(error $(VK_SDK_MISSING))
  endif
  CPPFLAGS := $(VK_CPPFLAGS) $(EXTRA_CPPFLAGS)
  LDLIBS   := $(VK_LDLIBS)
else
  CPPFLAGS := $(GL_CPPFLAGS) $(EXTRA_CPPFLAGS)
  LDLIBS   := $(GL_LDLIBS)
endif

LIB_FILE   := $(LIBDIR)/lib$(LIB).so
OBJS       := $(call src_to_obj,$(SRCS))
EXTRA_OBJS := $(call src_to_obj,$(OBJ_ONLY_SRCS))
DEP_LIBS   := $(LIB_DEPS_$(LIB))
DEP_FILES  := $(foreach l,$(DEP_LIBS),$(LIBDIR)/lib$(l).so)

.PHONY: all clean FORCE
.DEFAULT_GOAL := all

all: $(LIB_FILE) $(EXTRA_OBJS)

# --no-undefined: report missing system libraries here and not in every project
# -Bsymbolic-functions: calls inside the library always stay inside the library,
#                       even if a project defines a function with the same name
$(LIB_FILE): $(OBJS) $(DEP_FILES)
	@mkdir -p $(@D)
	$(CXX) -shared -o $@ $(OBJS) -Wl,--no-undefined -Wl,-Bsymbolic-functions \
		-L$(LIBDIR) $(addprefix -l,$(DEP_LIBS)) -Wl,-rpath,'$$ORIGIN' $(LDLIBS)

ifndef NO_LIB_BUILD
$(DEP_FILES): FORCE
	+@$(MAKE) -C $(LIB_DIR_$(patsubst lib%.so,%,$(notdir $@)))
endif

FORCE:

clean:
	rm -rf $(OBJDIR) $(LIB_FILE)

-include $(OBJS:.o=.d) $(EXTRA_OBJS:.o=.d)
