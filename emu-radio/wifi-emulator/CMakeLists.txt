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
project(wifi_emulator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

pkg_search_module(NS3-wifi REQUIRED libns3.26-wifi-optimized libns3.26-wifi libns3.26-wifi-debug libns3.25-wifi-optimized libns3.25-wifi libns3.25-wifi-debug libns3.24.1-wifi-optimized libns3.24.1-wifi libns3.24.1-wifi-debug)

set(WiFi_Ns3_LIBRARIES ${Ns3_LIBRARIES} ${NS3-wifi_LIBRARIES})
set(WiFi_Ns3_INCLUDE_DIRECTORIES ${NS3-wifi_INCLUDE_DIRS})

include_directories(${WiFi_Ns3_INCLUDE_DIRECTORIES})

set(SOURCE_FILES
        src/wifi_main.cc
        src/wifi-emulator.cc
        src/wifi-emulator.h)

add_executable(wifi_emulator ${SOURCE_FILES} ${COMMON_FILES})
target_link_libraries(wifi_emulator ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${WiFi_Ns3_LIBRARIES})

install(TARGETS wifi_emulator DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
