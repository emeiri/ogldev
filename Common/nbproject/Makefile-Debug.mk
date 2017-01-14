#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/FreetypeGL/font-manager.o \
	${OBJECTDIR}/FreetypeGL/font_shader.o \
	${OBJECTDIR}/FreetypeGL/freetypeGL.o \
	${OBJECTDIR}/FreetypeGL/texture-atlas.o \
	${OBJECTDIR}/FreetypeGL/texture-font.o \
	${OBJECTDIR}/FreetypeGL/texture-glyph.o \
	${OBJECTDIR}/FreetypeGL/vector.o \
	${OBJECTDIR}/FreetypeGL/vertex-buffer.o \
	${OBJECTDIR}/camera.o \
	${OBJECTDIR}/cubemap_texture.o \
	${OBJECTDIR}/glut_backend.o \
	${OBJECTDIR}/io_buffer.o \
	${OBJECTDIR}/math_3d.o \
	${OBJECTDIR}/ogldev_app.o \
	${OBJECTDIR}/ogldev_atb.o \
	${OBJECTDIR}/ogldev_backend.o \
	${OBJECTDIR}/ogldev_basic_lighting.o \
	${OBJECTDIR}/ogldev_basic_mesh.o \
	${OBJECTDIR}/ogldev_glfw_backend.o \
	${OBJECTDIR}/ogldev_shadow_map_fbo.o \
	${OBJECTDIR}/ogldev_skinned_mesh.o \
	${OBJECTDIR}/ogldev_texture.o \
	${OBJECTDIR}/ogldev_util.o \
	${OBJECTDIR}/pipeline.o \
	${OBJECTDIR}/random_texture.o \
	${OBJECTDIR}/technique.o


# C Compiler Flags
CFLAGS=`pkg-config --cflags freetype2` 

# CC Compiler Flags
CCFLAGS=`pkg-config --cflags ImageMagick++ freetype2 assimp` -fPIC 
CXXFLAGS=`pkg-config --cflags ImageMagick++ freetype2 assimp` -fPIC 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a

${OBJECTDIR}/FreetypeGL/font-manager.o: FreetypeGL/font-manager.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/font-manager.o FreetypeGL/font-manager.c

${OBJECTDIR}/FreetypeGL/font_shader.o: FreetypeGL/font_shader.cpp 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/font_shader.o FreetypeGL/font_shader.cpp

${OBJECTDIR}/FreetypeGL/freetypeGL.o: FreetypeGL/freetypeGL.cpp 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/freetypeGL.o FreetypeGL/freetypeGL.cpp

${OBJECTDIR}/FreetypeGL/texture-atlas.o: FreetypeGL/texture-atlas.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/texture-atlas.o FreetypeGL/texture-atlas.c

${OBJECTDIR}/FreetypeGL/texture-font.o: FreetypeGL/texture-font.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/texture-font.o FreetypeGL/texture-font.c

${OBJECTDIR}/FreetypeGL/texture-glyph.o: FreetypeGL/texture-glyph.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/texture-glyph.o FreetypeGL/texture-glyph.c

${OBJECTDIR}/FreetypeGL/vector.o: FreetypeGL/vector.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/vector.o FreetypeGL/vector.c

${OBJECTDIR}/FreetypeGL/vertex-buffer.o: FreetypeGL/vertex-buffer.c 
	${MKDIR} -p ${OBJECTDIR}/FreetypeGL
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FreetypeGL/vertex-buffer.o FreetypeGL/vertex-buffer.c

${OBJECTDIR}/camera.o: camera.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/camera.o camera.cpp

${OBJECTDIR}/cubemap_texture.o: cubemap_texture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cubemap_texture.o cubemap_texture.cpp

${OBJECTDIR}/glut_backend.o: glut_backend.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/glut_backend.o glut_backend.cpp

${OBJECTDIR}/io_buffer.o: io_buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/io_buffer.o io_buffer.cpp

${OBJECTDIR}/math_3d.o: math_3d.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/math_3d.o math_3d.cpp

${OBJECTDIR}/ogldev_app.o: ogldev_app.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_app.o ogldev_app.cpp

${OBJECTDIR}/ogldev_atb.o: ogldev_atb.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_atb.o ogldev_atb.cpp

${OBJECTDIR}/ogldev_backend.o: ogldev_backend.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_backend.o ogldev_backend.cpp

${OBJECTDIR}/ogldev_basic_lighting.o: ogldev_basic_lighting.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_basic_lighting.o ogldev_basic_lighting.cpp

${OBJECTDIR}/ogldev_basic_mesh.o: ogldev_basic_mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_basic_mesh.o ogldev_basic_mesh.cpp

${OBJECTDIR}/ogldev_glfw_backend.o: ogldev_glfw_backend.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_glfw_backend.o ogldev_glfw_backend.cpp

${OBJECTDIR}/ogldev_shadow_map_fbo.o: ogldev_shadow_map_fbo.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_shadow_map_fbo.o ogldev_shadow_map_fbo.cpp

${OBJECTDIR}/ogldev_skinned_mesh.o: ogldev_skinned_mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_skinned_mesh.o ogldev_skinned_mesh.cpp

${OBJECTDIR}/ogldev_texture.o: ogldev_texture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_texture.o ogldev_texture.cpp

${OBJECTDIR}/ogldev_util.o: ogldev_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ogldev_util.o ogldev_util.cpp

${OBJECTDIR}/pipeline.o: pipeline.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pipeline.o pipeline.cpp

${OBJECTDIR}/random_texture.o: random_texture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/random_texture.o random_texture.cpp

${OBJECTDIR}/technique.o: technique.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -IFreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/technique.o technique.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcommon.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
