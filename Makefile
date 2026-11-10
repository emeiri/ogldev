# ---------------------------------------------------------------------------
#  Master build for Linux
#
#  make          build the libraries, then all the projects in parallel
#  make clean    clean everything
#
#  Every project directory has its own Makefile and can be built on its own
#  by running 'make' inside it. See Make/project.mk.
#  Each project's output is saved to <project>/build.log.
# ---------------------------------------------------------------------------

include Make/config.mk

LIB_DIRS := Common DemoLITION Vulkan/VulkanCore

# Every directory whose Makefile includes Make/project.mk is a project
PROJECTS := $(shell grep -rl --include=Makefile --exclude-dir=.git --exclude-dir=build "Make/project.mk" .)
PROJECTS := $(sort $(patsubst ./%/Makefile,%,$(filter ./%/Makefile,$(PROJECTS))))

ifeq ($(VULKAN_SDK),)
  SKIPPED  := $(filter Vulkan/%,$(PROJECTS))
  PROJECTS := $(filter-out Vulkan/%,$(PROJECTS))
  LIB_DIRS := $(filter-out Vulkan/%,$(LIB_DIRS))
endif

STATE := $(ROOT)/build/master

.PHONY: all start libs clean $(PROJECTS) $(addprefix lib/,$(LIB_DIRS))

all: $(PROJECTS)
	@Make/report.sh $(STATE)

start:
	@rm -rf $(STATE) && mkdir -p $(STATE)
	@date +%s > $(STATE)/start
	@printf "%s" "$(SKIPPED)" | tr " " "\n" | grep . > $(STATE)/skipped || true
	@echo "Building $(words $(LIB_DIRS)) libraries and $(words $(PROJECTS)) projects using $(NPROC) cores"

# Phase 1: the libraries
libs: $(addprefix lib/,$(LIB_DIRS))

$(addprefix lib/,$(LIB_DIRS)): start
	+@Make/build_one.sh lib $(patsubst lib/%,%,$@) $(STATE) 0

lib/DemoLITION: lib/Common

# Phase 2: the projects, after all the libraries are done
$(PROJECTS): libs
	+@Make/build_one.sh project $@ $(STATE) $(words $(PROJECTS))

clean:
	@for d in $(LIB_DIRS) $(PROJECTS); do $(MAKE) -s -C $$d clean NO_LIB_BUILD=1; done
	rm -rf $(ROOT)/build $(LIBDIR)
