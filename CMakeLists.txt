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
project(http-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Libicnet REQUIRED)
include_directories(${LIBICNET_INCLUDE_DIRS})

find_package(Threads REQUIRED)

find_package(Boost 1.53.0 COMPONENTS regex system thread filesystem date_time REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${LIBICNET_INCLUDE_DIR})

set(SOURCE_FILES
        main.cc
        http-server/http_server.cc
        http-server/http_server.h
        http-server/response.cc
        http-server/response.h
        http-server/common.h
        http-server/socket_response.cc
        http-server/socket_response.h
        http-server/icn_response.cc
        http-server/icn_response.h
        http-server/content.cc
        http-server/content.h
        http-server/request.cc
        http-server/request.h
        http-server/icn_request.cc
        http-server/icn_request.h
        http-server/socket_request.cc
        http-server/socket_request.h
        http-server/configuration.cc
        http-server/configuration.h)

add_executable(http-server ${SOURCE_FILES})
target_link_libraries(http-server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBICNET_LIBRARY})

install(TARGETS http-server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
