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

cmake_minimum_required(VERSION 3.2)
project(Libicnet)
set(CMAKE_CXX_STANDARD 11)

set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)

## Set build folders
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CCNX_API icnet/ccnx)
set(CP_API icnet/transport)
set(COMMON_INCLUDES icnet/common-includes)
set(APPS apps)

option(BUILD_APPS "Build apps" ON)
message("Build apps programs: " ${BUILD_TESTS})

message("install prefix: " ${CMAKE_INSTALL_PREFIX})
set(raaqm_config_path ${CMAKE_INSTALL_PREFIX}/etc/consumer.conf)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file("${PROJECT_SOURCE_DIR}/config.hpp.in"
        "${CMAKE_BINARY_DIR}/config.hpp")

include_directories(
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/${CP_API}
        ${PROJECT_SOURCE_DIR}/${CCNX_API}
        ${PROJECT_SOURCE_DIR}/${COMMON_INCLUDES}
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(LongBow REQUIRED)
include_directories(${LONGBOW_INCLUDE_DIRS})

find_package(Libparc REQUIRED)
include_directories(${LIBPARC_INCLUDE_DIRS})

find_package(CCNX_Common REQUIRED)
include_directories(${CCNX_COMMON_INCLUDE_DIRS})

find_package(CCNX_Transport_Rta REQUIRED)
include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

find_package(CCNX_Portal REQUIRED)
include_directories(${CCNX_PORTAL_INCLUDE_DIRS})

find_package(Threads REQUIRED)
include_directories(${CMAKE_THREADS_INCLUDE_DIRS})

find_package(Threads REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE off)

if(ANDROID_API)
  include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/include)
  include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include)
  set(Boost_LIBRARIES $ENV{CCNX_DEPENDENCIES}/lib/libboost_regex.a $ENV{CCNX_DEPENDENCIES}/lib/libboost_system.a $ENV{CCNX_DEPENDENCIES}/lib/libboost_thread.a $ENV{CCNX_DEPENDENCIES}/lib/libboost_filesystem.a $ENV{CCNX_DEPENDENCIES}/lib/libboost_date_time.a $ENV{CCNX_DEPENDENCIES}/lib/libboost_chrono.a)
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} ${ANDROID_C_FLAGS} -std=c++11 -Wall -fpermissive -O3")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fpermissive -O3")
  find_package(Boost 1.53.0 COMPONENTS system REQUIRED)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  if (BUILD_APPS)
    set(SUBFOLDERS ${APPS})
  endif (BUILD_APPS)
endif (ANDROID_API)

set(LIBRARIES
        ${CCNX_PORTAL_LIBRARIES}
        ${CCNX_TRANSPORT_RTA_LIBRARIES}
        ${CCNX_COMMON_LIBRARIES}
        ${LIBPARC_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${LONGBOW_LIBRARIES})

set(SUBFOLDERS ${SUBFOLDERS} icnet)

subdirs(${SUBFOLDERS})

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

SET(VENDOR "Cisco Systems" CACHE STRING "Vendor")
SET(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
SET(DISTRIBUTION "xenial" CACHE STRING "Distribution")
SET(ARCHITECTURE "amd64" CACHE STRING "Architecture")
SET(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
SET(PACKAGE_VERSION "1.0" CACHE STRING "Version")
SET(BUILD_NUMBER "1" CACHE STRING "Build Number")
STRING(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_PACKAGE_CONTACT ${CONTACT})

if(DEB_PACKAGE)
    SET(TYPE "DEBIAN")
    SET(GENERATOR "DEB")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}~${DISTRIBUTION}_${ARCHITECTURE}")
    SET(CPACK_${TYPE}_PACKAGE_DEPENDS "longbow (>= 1.0), libevent-2.0-5, libssl1.0.0, libparc (>= 1.0), libccnx-common (>= 1.0), libccnx-transport-rta (>= 1.0), libccnx-portal (>= 1.0), libboost-system-dev (>= 1.58)")
elseif(RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent >= 2.0, openssl >= 1.0, libparc >= 1.0, libccnx-common >= 1.0, libccnx-transport-rta >= 1.0, libccnx-portal >= 1.0, boost-system >= 1.53")
else()
    RETURN()
endif()

SET(CPACK_GENERATOR ${GENERATOR})
SET(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
SET(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
SET(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
SET(CPACK_${TYPE}_PACKAGE_RELEASE 1)
SET(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "This library is designed to provide a transport layer for applications willing to communicate using an ICN protocol stack.")
SET(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Libicnet")

INCLUDE(CPack)
