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
install(FILES ${PROJECT_SOURCE_DIR}/fading-traces/fading_trace_EPA_3kmph.fad DESTINATION share/lte-emulator)
install(FILES ${PROJECT_SOURCE_DIR}/fading-traces/fading_trace_ETU_3kmph.fad DESTINATION share/lte-emulator)
install(FILES ${PROJECT_SOURCE_DIR}/fading-traces/fading_trace_EVA_60kmph.fad DESTINATION share/lte-emulator)
install(FILES ${PROJECT_SOURCE_DIR}/fading-traces/fading_trace_generator.m DESTINATION share/lte-emulator)

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
set(BUILD_NUMBER "1" CACHE STRING "Build Number")
set(PACKAGE_NAME lte-emulator)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VENDOR ${VENDOR})
set(CPACK_PACKAGE_CONTACT ${CONTACT})

# Get the version
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/version
                OUTPUT_VARIABLE PACKAGE_VERSION)

if (PACKAGE_VERSION)
  string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)
else()
  set(PACKAGE_VERSION 1.0)
endif()

if(DEB_PACKAGE)
    set(TYPE "DEBIAN")
    set(GENERATOR "DEB")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}")
    set(CPACK_${TYPE}_PACKAGE_DEPENDS "libns3sx-3v5, ns3sx, libboost-system1.58.0")
elseif(RPM_PACKAGE)
    set(TYPE "RPM")
    set(GENERATOR "RPM")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}")
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    set(CPACK_${TYPE}_PACKAGE_REQUIRES "")
else()
    return()
endif()

set(CPACK_GENERATOR ${GENERATOR})
set(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
set(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
set(CPACK_${TYPE}_PACKAGE_RELEASE 1)
set(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "LTE channel emulator.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Vicn")

include(CPack)
