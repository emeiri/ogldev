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
CND_PLATFORM=GNU-Linux
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
	${OBJECTDIR}/tutorial50.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig glfw3` 
CXXFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig glfw3` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../Lib ../Common/dist/Debug/GNU-Linux/libcommon.a -lglut -Wl,-rpath,'../CommonVulkan/dist/Debug/GNU-Linux' -L../CommonVulkan/dist/Debug/GNU-Linux -lCommonVulkan

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50: ../Common/dist/Debug/GNU-Linux/libcommon.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50: ../CommonVulkan/dist/Debug/GNU-Linux/libCommonVulkan.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50 ${OBJECTFILES} ${LDLIBSOPTIONS} `pkg-config --libs glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 x11-xcb vulkan`

${OBJECTDIR}/tutorial50.o: tutorial50.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DVULKAN -I../Include -I../Common/FreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tutorial50.o tutorial50.cpp

# Subprojects
.build-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug
	cd ../CommonVulkan && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} -r ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCommonVulkan.so
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial50

# Subprojects
.clean-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../CommonVulkan && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
