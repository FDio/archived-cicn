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
project(lte_emulator)

pkg_search_module(NS3-lte REQUIRED libns3.26-lte-optimized libns3.26-lte libns3.26-lte-debug libns3.25-lte-optimized libns3.25-lte libns3.25-lte-debug libns3.24.1-lte-optimized libns3.24.1-lte libns3.24.1-lte-debug)
pkg_search_module(NS3-internet REQUIRED libns3.26-internet-optimized libns3.26-internet libns3.26-internet-debug libns3.25-internet-optimized libns3.25-internet libns3.25-internet-debug libns3.24.1-internet-optimized libns3.24.1-internet libns3.24.1-internet-debug)
pkg_search_module(NS3-csma REQUIRED libns3.26-csma-optimized libns3.26-csma libns3.26-csma-debug libns3.25-csma-optimized libns3.25-csma libns3.25-csma-debug libns3.24.1-csma-optimized libns3.24.1-csma libns3.24.1-csma-debug)


set(Lte_Ns3_LIBRARIES ${Ns3_LIBRARIES} ${NS3-lte_LIBRARIES} ${NS3-csma_LIBRARIES})
set(Lte_Ns3_INCLUDE_DIRECTORIES ${NS3-lte_INCLUDE_DIRS} ${NS3-csma_INCLUDE_DIRS})

include_directories(${Lte_Ns3_INCLUDE_DIRECTORIES} src)

set(SOURCE_FILES
        lte_main.cc
        extensions/lte-tap-helper.h
        extensions/lte-tap-helper.cc
        extensions/lte-tap-ue-net-device.h
        extensions/lte-tap-ue-net-device.cc
        extensions/tap-point-to-point-epc-helper.h
        extensions/tap-point-to-point-epc-helper.cc
        src/lte-emulator.cc
        src/lte-emulator.h)

add_executable(lte_emulator ${SOURCE_FILES} ${COMMON_FILES})
target_link_libraries(lte_emulator ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Lte_Ns3_LIBRARIES})

install(TARGETS lte_emulator DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
