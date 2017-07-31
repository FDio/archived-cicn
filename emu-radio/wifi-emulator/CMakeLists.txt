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

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
set(BUILD_NUMBER "1" CACHE STRING "Build Number")
set(PACKAGE_NAME wifi-emulator)

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
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Wifi N channel emulator.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Vicn")

include(CPack)
