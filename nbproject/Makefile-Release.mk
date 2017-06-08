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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/codecs/datapathid.o \
	${OBJECTDIR}/codecs/ofp_async_config.o \
	${OBJECTDIR}/codecs/ofp_flow_mod.o \
	${OBJECTDIR}/codecs/ofp_header.o \
	${OBJECTDIR}/codecs/ofp_hello.o \
	${OBJECTDIR}/codecs/ofp_port_status.o \
	${OBJECTDIR}/codecs/ofp_switch_features.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/tcp_session.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/codecs/datapathid.o: codecs/datapathid.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/datapathid.o codecs/datapathid.cpp

${OBJECTDIR}/codecs/ofp_async_config.o: codecs/ofp_async_config.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_async_config.o codecs/ofp_async_config.cpp

${OBJECTDIR}/codecs/ofp_flow_mod.o: codecs/ofp_flow_mod.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_flow_mod.o codecs/ofp_flow_mod.cpp

${OBJECTDIR}/codecs/ofp_header.o: codecs/ofp_header.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_header.o codecs/ofp_header.cpp

${OBJECTDIR}/codecs/ofp_hello.o: codecs/ofp_hello.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_hello.o codecs/ofp_hello.cpp

${OBJECTDIR}/codecs/ofp_port_status.o: codecs/ofp_port_status.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_port_status.o codecs/ofp_port_status.cpp

${OBJECTDIR}/codecs/ofp_switch_features.o: codecs/ofp_switch_features.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_switch_features.o codecs/ofp_switch_features.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/tcp_session.o: tcp_session.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tcp_session.o tcp_session.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
