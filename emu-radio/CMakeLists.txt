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

cmake_minimum_required(VERSION 3.4)
project(Radio_Emulation)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

option(LTE "Build LTE emulator" ON)
option(WIFI "Build WiFi emulator" ON)

message("Build LTE emulator " ${LTE})
message("Build WiFi emulator " ${WIFI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(FindPkgConfig)

find_package(LibXml2 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.53.0 COMPONENTS system REQUIRED)

pkg_search_module(NS3-core REQUIRED libns3.26-core-optimized libns3.26-core libns3.26-core-debug libns3.25-core-optimized libns3.25-core libns3.25-core-debug libns3.24.1-core-optimized libns3.24.1-core-debug libns3.24.1-core)
pkg_search_module(NS3-network REQUIRED libns3.26-network-optimized libns3.26-network libns3.26-network-debug libns3.25-network-optimized libns3.25-network libns3.25-network-debug libns3.24.1-network-optimized libns3.24.1-network libns3.24.1-network-debug)
pkg_search_module(NS3-tap-bridge REQUIRED libns3.26-tap-bridge-optimized libns3.26-tap-bridge libns3.26-tap-bridge-debug libns3.25-tap-bridge-optimized libns3.25-tap-bridge libns3.25-tap-bridge-debug libns3.24.1-tap-bridge-optimized libns3.24.1-tap-bridge libns3.24.1-tap-bridge-debug)
pkg_search_module(NS3-mobility REQUIRED libns3.26-mobility-optimized libns3.26-mobility libns3.26-mobility-debug libns3.25-mobility-optimized libns3.25-mobility libns3.25-mobility-debug libns3.24.1-mobility-optimized libns3.24.1-mobility libns3.24.1-mobility-debug)

set(Ns3_LIBRARIES ${NS3-core_LIBRARIES} ${NS3-network_LIBRARIES} ${NS3-tap-bridge_LIBRARIES} ${NS3-mobility_LIBRARIES})
set(Ns3_INCLUDE_DIRECTORIES ${NS3-core_INCLUDE_DIRS} ${NS3-network_INCLUDE_DIRS} ${NS3-tap-bridge_INCLUDE_DIRS} ${NS3-mobility_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/common-library/headers ${Ns3_INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${Ns3_INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/common-library)
include_directories(${WEBSOCKETPP_INCLUDE_DIR})

set(COMMON_FILES
        ${PROJECT_SOURCE_DIR}/common-library/communication-protocol.cc
        ${PROJECT_SOURCE_DIR}/common-library/communication-protocol.h
        ${PROJECT_SOURCE_DIR}/common-library/tcp-server.cc
        ${PROJECT_SOURCE_DIR}/common-library/tcp-server.h
        ${PROJECT_SOURCE_DIR}/common-library/connection-pool.cc
        ${PROJECT_SOURCE_DIR}/common-library/connection-pool.h
        ${PROJECT_SOURCE_DIR}/common-library/websocket-server.cc
        ${PROJECT_SOURCE_DIR}/common-library/websocket-server.h
        ${PROJECT_SOURCE_DIR}/common-library/query.cc
        ${PROJECT_SOURCE_DIR}/common-library/query.h)

if (WIFI)
    set(SUBFOLDERS ${SUBFOLDERS} wifi-emulator)
endif ()

if (LTE)
    set(SUBFOLDERS ${SUBFOLDERS} lte-emulator)
endif ()

if (NOT WIFI AND NOT LTE)
    message(FATAL_ERROR "You should build at least one emulator. CMake will exit.")
endif ()

subdirs(${SUBFOLDERS})
