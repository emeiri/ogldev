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
	${OBJECTDIR}/mesh.o \
	${OBJECTDIR}/skybox.o \
	${OBJECTDIR}/skybox_technique.o \
	${OBJECTDIR}/tutorial25.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig` 
CXXFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lglut ../Common/dist/Debug/GNU-Linux-x86/libcommon.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial25

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial25: ../Common/dist/Debug/GNU-Linux-x86/libcommon.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial25: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial25 ${OBJECTFILES} ${LDLIBSOPTIONS} `pkg-config --libs glew ImageMagick++ assimp gl freetype2 fontconfig`

${OBJECTDIR}/mesh.o: mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -I../Common/FreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mesh.o mesh.cpp

${OBJECTDIR}/skybox.o: skybox.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -I../Common/FreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/skybox.o skybox.cpp

${OBJECTDIR}/skybox_technique.o: skybox_technique.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -I../Common/FreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/skybox_technique.o skybox_technique.cpp

${OBJECTDIR}/tutorial25.o: tutorial25.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -I../Include -I../Common/FreetypeGL -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tutorial25.o tutorial25.cpp

# Subprojects
.build-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tutorial25

# Subprojects
.clean-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
