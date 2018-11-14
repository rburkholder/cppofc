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
	${OBJECTDIR}/bridge.o \
	${OBJECTDIR}/codecs/datapathid.o \
	${OBJECTDIR}/codecs/ofp_async_config.o \
	${OBJECTDIR}/codecs/ofp_flow_mod.o \
	${OBJECTDIR}/codecs/ofp_header.o \
	${OBJECTDIR}/codecs/ofp_hello.o \
	${OBJECTDIR}/codecs/ofp_packet_out.o \
	${OBJECTDIR}/codecs/ofp_port_status.o \
	${OBJECTDIR}/codecs/ofp_switch_features.o \
	${OBJECTDIR}/mac.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/protocol/ethernet.o \
	${OBJECTDIR}/protocol/ipv4.o \
	${OBJECTDIR}/protocol/ipv4/arp.o \
	${OBJECTDIR}/protocol/ipv4/dhcp.o \
	${OBJECTDIR}/protocol/ipv4/tcp.o \
	${OBJECTDIR}/protocol/ipv4/udp.o \
	${OBJECTDIR}/protocol/ipv6.o \
	${OBJECTDIR}/protocol/vlan.o \
	${OBJECTDIR}/tcp_session.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -lboost_system-mt -lzmqpp -lzmq -lboost_thread-mt -lboost_log_setup-mt -lboost_log-mt

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cppofc ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/bridge.o: bridge.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/bridge.o bridge.cpp

${OBJECTDIR}/codecs/datapathid.o: codecs/datapathid.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/datapathid.o codecs/datapathid.cpp

${OBJECTDIR}/codecs/ofp_async_config.o: codecs/ofp_async_config.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_async_config.o codecs/ofp_async_config.cpp

${OBJECTDIR}/codecs/ofp_flow_mod.o: codecs/ofp_flow_mod.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_flow_mod.o codecs/ofp_flow_mod.cpp

${OBJECTDIR}/codecs/ofp_header.o: codecs/ofp_header.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_header.o codecs/ofp_header.cpp

${OBJECTDIR}/codecs/ofp_hello.o: codecs/ofp_hello.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_hello.o codecs/ofp_hello.cpp

${OBJECTDIR}/codecs/ofp_packet_out.o: codecs/ofp_packet_out.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_packet_out.o codecs/ofp_packet_out.cpp

${OBJECTDIR}/codecs/ofp_port_status.o: codecs/ofp_port_status.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_port_status.o codecs/ofp_port_status.cpp

${OBJECTDIR}/codecs/ofp_switch_features.o: codecs/ofp_switch_features.cpp
	${MKDIR} -p ${OBJECTDIR}/codecs
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/codecs/ofp_switch_features.o codecs/ofp_switch_features.cpp

${OBJECTDIR}/mac.o: mac.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mac.o mac.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/protocol/ethernet.o: protocol/ethernet.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ethernet.o protocol/ethernet.cpp

${OBJECTDIR}/protocol/ipv4.o: protocol/ipv4.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv4.o protocol/ipv4.cpp

${OBJECTDIR}/protocol/ipv4/arp.o: protocol/ipv4/arp.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol/ipv4
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv4/arp.o protocol/ipv4/arp.cpp

${OBJECTDIR}/protocol/ipv4/dhcp.o: protocol/ipv4/dhcp.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol/ipv4
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv4/dhcp.o protocol/ipv4/dhcp.cpp

${OBJECTDIR}/protocol/ipv4/tcp.o: protocol/ipv4/tcp.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol/ipv4
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv4/tcp.o protocol/ipv4/tcp.cpp

${OBJECTDIR}/protocol/ipv4/udp.o: protocol/ipv4/udp.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol/ipv4
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv4/udp.o protocol/ipv4/udp.cpp

${OBJECTDIR}/protocol/ipv6.o: protocol/ipv6.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/ipv6.o protocol/ipv6.cpp

${OBJECTDIR}/protocol/vlan.o: protocol/vlan.cpp
	${MKDIR} -p ${OBJECTDIR}/protocol
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/protocol/vlan.o protocol/vlan.cpp

${OBJECTDIR}/tcp_session.o: tcp_session.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -D_DEBUG -DBOOST_LOG_DYN_LINK -I/usr/local/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tcp_session.o tcp_session.cpp

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
