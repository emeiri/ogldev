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
	${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan.o \
	${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan_core.o \
	${OBJECTDIR}/_ext/7daaf93a/ogldev_win32_control.o \
	${OBJECTDIR}/_ext/7daaf93a/ogldev_xcb_control.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-fPIC
CXXFLAGS=-fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../Common/dist/Debug/GNU-Linux/libcommon.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCommonVulkan.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCommonVulkan.${CND_DLIB_EXT}: ../Common/dist/Debug/GNU-Linux/libcommon.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCommonVulkan.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libCommonVulkan.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan.o: ../Common/ogldev_vulkan.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DVULKAN -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan.o ../Common/ogldev_vulkan.cpp

${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan_core.o: ../Common/ogldev_vulkan_core.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DVULKAN -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/ogldev_vulkan_core.o ../Common/ogldev_vulkan_core.cpp

${OBJECTDIR}/_ext/7daaf93a/ogldev_win32_control.o: ../Common/ogldev_win32_control.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DVULKAN -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/ogldev_win32_control.o ../Common/ogldev_win32_control.cpp

${OBJECTDIR}/_ext/7daaf93a/ogldev_xcb_control.o: ../Common/ogldev_xcb_control.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DVULKAN -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/ogldev_xcb_control.o ../Common/ogldev_xcb_control.cpp

# Subprojects
.build-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../Common && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
