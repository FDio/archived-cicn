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
project(libicnet)
set(CMAKE_CXX_STANDARD 11)

set(CP_Api_VERSION_MAJOR 0)
set(CP_Api_VERSION_MINOR 1)

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

message(${LIBPARC_INCLUDE_DIRS})

find_package(CCNX_Common REQUIRED)
include_directories(${CCNX_COMMON_INCLUDE_DIRS})

find_package(CCNX_Transport_Rta REQUIRED)
include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

find_package(CCNX_Portal REQUIRED)
include_directories(${CCNX_PORTAL_INCLUDE_DIRS})

find_package(Threads REQUIRED)
include_directories(${CMAKE_THREADS_INCLUDE_DIRS})

find_package(Threads REQUIRED)

find_package(Boost 1.54.0 COMPONENTS system REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

set(CMAKE_VERBOSE_MAKEFILE off)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fpermissive -O3")

set(LIBRARIES
        ${CCNX_PORTAL_LIBRARIES}
        ${CCNX_TRANSPORT_RTA_LIBRARIES}
        ${CCNX_COMMON_LIBRARIES}
        ${LIBPARC_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${LONGBOW_LIBRARIES})

if (BUILD_APPS)
    set(SUBFOLDERS ${APPS})
endif (BUILD_APPS)

set(SUBFOLDERS ${SUBFOLDERS} icnet)

subdirs(${SUBFOLDERS})
