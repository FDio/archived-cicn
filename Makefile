 #############################################################################
 # Copyright (c) 2017 Cisco and/or its affiliates.
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at:
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 ##############################################################################

DISTILLERY_VERSION=2.0

default.target: help

all: init_depend install-all init_qt

##############################################################
# Variables
#
# Set some variables
DISTILLERY_STAMP=.distillery.stamp
REBUILD_DEPENDS=

##############################################################
# Load the configuration
#
# For more information please see config.default.mk
#
DISTILLERY_CONFIG_DIR ?= config

##DISTILLERY_DEFAULT_CONFIG ?= ${DISTILLERY_CONFIG_DIR}/config.mk
##DISTILLERY_LOCAL_CONFIG   ?= ${DISTILLERY_CONFIG_DIR}/local/config.mk
DISTILLERY_USER_CONFIG    ?= ${DISTILLERY_CONFIG_DIR}/config.mk

ifneq (,$(wildcard ${DISTILLERY_USER_CONFIG}))
    include ${DISTILLERY_USER_CONFIG}
    REBUILD_DEPENDS+=${DISTILLERY_USER_CONFIG}
else
    DISTILLERY_USER_CONFIG+="[Not Found]"
endif

ifneq (,$(wildcard ${DISTILLERY_LOCAL_CONFIG}))
    include ${DISTILLERY_LOCAL_CONFIG}
    REBUILD_DEPENDS+=${DISTILLERY_LOCAL_CONFIG}
endif

include ${DISTILLERY_DEFAULT_CONFIG}


##############################################################
# Set the paths
#
# PATH: add our install dir, build dependencies and system dependencies
# LD_RUN_PATH: add our install dir

export PATH := $(DISTILLERY_INSTALL_DIR)/bin:$(DISTILLERY_TOOLS_DIR)/bin:$(PATH)
#export LD_RUN_PATH := $(DISTILLERY_INSTALL_DIR)/lib
#export LD_LIBRARY_PATH := $(DISTILLERY_INSTALL_DIR)/lib
export CCNX_HOME
export FOUNDATION_HOME


##############################################################
# Modules
#
# Load the modules config. Please refer to that file for more information
DISTILLERY_MODULES_DIR=${DISTILLERY_CONFIG_DIR}/modules

# The modules variable is a list of modules. It will be populated by the
# modules config files.
modules=
modules_dir=
    include config/modules/000-distillery-update.mk
	include config/modules/000-gitModule.mk
	include config/modules/001-modules.mk
	include config/modules/002-cmake-modules.mk
	include config/modules/002-make-modules.mk
	include config/modules/100-distillery.mk
	include config/modules/110-longbow.mk
	include config/modules/120-libparc.mk
	include config/modules/210-libccnx-common.mk
	include config/modules/220-libccnx-transport-rta.mk
	include config/modules/230-libccnx-portal.mk
	include config/modules/510-Metis.mk
	include config/modules/610-libicnet.mk
	include config/modules/610-libdash.mk

# Load user defined modules
DISTILLERY_USER_MODULES_DIR=${DISTILLERY_USER_CONFIG_DIR}/modules
ifneq (,$(wildcard ${DISTILLERY_USER_MODULES_DIR}))
    include ${DISTILLERY_USER_MODULES_DIR}/*.mk
else
    DISTILLERY_USER_MODULES_DIR+="[Not Found]"
endif

ifdef ${DISTILLERY_LOCAL_MODULES_DIR}
    include ${DISTILLERY_LOCAL_MODULES_DIR}/*.mk
else
    DISTILLERY_LOCAL_MODULES_DIR="[Undefined]"
endif

install-all: install-directories ${modules}

init_depend:
	./scripts/init.sh ${ABI} ${DISTILLERY_INSTALL_DIR};
init_qt:
	./scripts/init_qt.sh
android_metis:
	./scripts/compile_androidmetis.sh
android_metis_debug:
	./scripts/compile_androidmetis.sh DEBUG
android_iget:
	./scripts/compile_androidiget.sh
android_iget_debug:
	./scripts/compile_androidiget.sh DEBUG
android_viper:
	./scripts/compile_androidviper.sh
android_iget_debug:
	./scripts/compile_androidviper.sh DEBUG

curl-clean:
	@rm -rf external/curl
	@rm -rf external/libcurl_android/obj
	@rm -rf external/libcurl_android/jni/libcurl/include
	@rm -rf external/libcurl_android/jni/libcurl/src
	@rm -rf external/libcurl_android/jni/libcurl/lib
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libcurl*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/curl

boost-clean:
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libboost*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/boost

openssl-clean:
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libssl.*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libcrypto.*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/openssl
	@rm -rf external/openssl-1.0.2k*
	@rm -rf external/crystax-ndk-10.3.2/sources/openssl/1.0.2k

crystax-clean:
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libxrystax.*

event-clean:
	@rm -rf external/libevent
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libevent*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/event2

crystaxndk-clean:
	@rm -rf external/crystax-ndk*

xml2-clean:
	@rm -rf external/libxml2
	@rm -rf external/libxml2_android/obj
	@rm -rf external/libxml2_android/jni/libxml2/*.c
	@rm -rf external/libxml2_android/jni/libxml2/*.h
	@rm -rf external/libxml2_android/jni/libxml2/include
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libxml2*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/libxml
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/win32config.h
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/wsockcompat.h
	
dependencies-clean: crystaxndk-clean event-clean crystax-clean openssl-clean boost-clean curl-clean xml2-clean
	
sdk-clean:
	@rm -rf sdk/android-sdk_*
	@rm -rf sdk/sdk
	
ndk-clean:
	@rm -rf sdk/android-ndk-*
	@rm -rf sdk/ndk-bundle
	
cmake-clean:
	@rm -rf cmake-*
	@rm -rf cmake
	
androidsdk-clean: sdk-clean ndk-clean cmake-clean

cframework-clean:
	@rm -rf ${DISTILLERY_BUILD_DIR}/cframework
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/liblongbow.*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/LongBow
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libparc.*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/parc
	
ccnxlibs-clean:
	@rm -rf src/ccnxlibs
	@rm -rf ${DISTILLERY_BUILD_DIR}/ccnxlibs
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libccnx-*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/ccnx/common
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/ccnx/transport
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/ccnx/api

sb-forwarder-clean:
	@rm -rf src/sb-forwarder
	@rm -rf ${DISTILLERY_BUILD_DIR}/sb-forwarder
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libmetis*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/ccnx/forwarder
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/config.h

libicnet-clean:
	@rm -rf src/libicnet
	@rm -rf ${DISTILLERY_BUILD_DIR}/libicnet
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libicnet*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/ccnx/icnet

libdash-clean:
	@rm -rf ${DISTILLERY_BUILD_DIR}/libdash
	@rm -rf ${DISTILLERY_INSTALL_DIR}/lib/libdash.*
	@rm -rf ${DISTILLERY_INSTALL_DIR}/include/libdash

qt-clean:
	@rm -rf qt/*
	@rm -rf ${DISTILLERY_BUILD_DIR}/qtav

all-clean: dependencies-clean cframework-clean ccnxlibs-clean sb-forwarder-clean libicnet-clean qt-clean

update:
	./scripts/update.sh

help:
	@echo "---- Basic build targets ----"
	@echo "make help			- This help message"
	@echo "make update			- git pull the different modules to the head of master"
	@echo "make all				- Download sdk, ndk and dependencies, configure, compile and install all software in DISTILLERY_INSTALL_DIR"
	@echo "make init_depend 	- Download sdk, ndk and dependencies, compile and install all dependencies in DISTILLERY_INSTALL"
	@echo "make install-all 	- Configure, compile and install all software in DISTILLERY_INSTALL_DIR"
	@echo "curl-clean			- Clean curl files and libs"
	@echo "boost-clean			- Clean boost files and libs"
	@echo "openssl-clean		- Clean opennssl files and libs"
	@echo "crystax-clean		- Clean crystax files and libs"
	@echo "event-clean			- Clean libevent files and libs"
	@echo "crystaxndk-clean		- Clean crystax ndk files"
	@echo "xml2-clean			- Clean libxml2 files and libs"
	@echo "dependencies-clean 	- Clean all dependencies files and libs"
	@echo "sdk-clean			- Clean sdk files"
	@echo "ndk-clean			- Clean ndk files"
	@echo "cmake-clean			- Clean cmake files"
	@echo "androidsdk-clean		- Clean sdk, ndk and cmake files"
	@echo "cframework-clean		- Clean cframework (libparc and longbow) files and libs"
	@echo "ccnxlibs-clean		- Clean ccnxlibs files and libs"
	@echo "sb-forwarder-clean	- Clean sb-forwarder (metis) files and libs"
	@echo "libicnet-clean		- Clean libicnet files and libs"
	@echo "libdash-clean		- Clean libdash files and libs"
	@echo "all-clean			- Clean	all files and libs"
	@echo "android_metis		- Build metis apk for android"
	@echo "android_metis_debug	- Build metis apk for android in debug mode"
	@echo "android_iget			- Build iGet apk for android apk in debug mode"
	@echo "android_iget_debug	- Build iGet apk for android apk"

${DISTILLERY_STAMP}: ${REBUILD_DEPENDS}
	touch $@

install-directories:
	@mkdir -p ${DISTILLERY_INSTALL_DIR}/include
	@mkdir -p ${DISTILLERY_INSTALL_DIR}/lib
	@mkdir -p ${DISTILLERY_INSTALL_DIR}/bin


.PHONY: dependencies