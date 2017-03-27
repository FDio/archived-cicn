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

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

SET(VENDOR "Cisco Systems" CACHE STRING "Vendor")
SET(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
SET(DISTRIBUTION "xenial" CACHE STRING "Distribution")
SET(ARCHITECTURE "amd64" CACHE STRING "Architecture")
SET(PACKAGE_MAINTAINER "Mauro Sardara" CACHE STRING "Maintainer")
SET(PACKAGE_VERSION "1.0" CACHE STRING "Version")
SET(BUILD_NUMBER "1" CACHE STRING "Build Number")
STRING(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_PACKAGE_CONTACT ${CONTACT})

if (DEB_PACKAGE)
    SET(TYPE "DEBIAN")
    SET(GENERATOR "DEB")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}~${DISTRIBUTION}_${ARCHITECTURE}")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent-2.0-5, libssl1.0.0, libparc >= 1.0, libccnx-common >= 1.0, libboost-system >= 1.53, libboost-regex >= 1.53, libboost-filesystem >= 1.53")
elseif (RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent >= 2.0, openssl >= 1.0, libparc >= 1.0, libccnx-common >= 1.0, boost-system >= 1.53, boost-regex >= 1.53, boost-filesystem >= 1.53")
else ()
    RETURN()
endif ()

SET(CPACK_GENERATOR ${GENERATOR})
SET(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
SET(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
SET(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
SET(CPACK_${TYPE}_PACKAGE_RELEASE 1)
SET(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Implementation of a HTTP server able to serve client requests using both TCP and ICN as transport protocol.")

INCLUDE(CPack)