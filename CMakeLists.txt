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
set(CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(ICNET "Link against Libicnet." ON)
option(HICNET "Link against Libhicnet." OFF)

if (HICNET)
    find_package(Libhicnet REQUIRED)
    set(TRANSPORT_LIBRARY HICNET)
    set(ICNET OFF)
    set(PREFIX "hicn-")
else()
    find_package(Libicnet REQUIRED)
    set(TRANSPORT_LIBRARY ICNET)
    set(PREFIX "")
endif()

configure_file("${PROJECT_SOURCE_DIR}/config.h.in"
        "${CMAKE_BINARY_DIR}/config.h")

include_directories(${LIB${TRANSPORT_LIBRARY}_INCLUDE_DIRS} http-server http-client)

find_package(Threads REQUIRED)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(Boost 1.53.0 COMPONENTS regex system filesystem REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ${CURL_INCLUDE_DIRS} ${LIB${TRANSPORT_LIBRARY}_INCLUDE_DIR})

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
    http-server/configuration.h
    http-client/http_client.cc
    http-client/http_client.h
    ${CMAKE_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS} -fpermissive")

add_executable(${PREFIX}http-server ${SOURCE_FILES})
target_link_libraries(${PREFIX}http-server ${LIB${TRANSPORT_LIBRARY}_LIBRARY} ${CURL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PREFIX}http-server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
set(BUILD_NUMBER "1" CACHE STRING "Build Number")
string(TOLOWER ${PREFIX}${CMAKE_PROJECT_NAME} PACKAGE_NAME)

set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VENDOR ${VENDOR})
set(CPACK_PACKAGE_CONTACT ${CONTACT})

# Get the version
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/version
                OUTPUT_VARIABLE PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)

string(TOLOWER ${TRANSPORT_LIBRARY} TRANSPORT_DEPENDENCY)

if (DEB_PACKAGE)
    set(TYPE "DEBIAN")
    set(GENERATOR "DEB")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_${TYPE}_PACKAGE_DEPENDS "lib${TRANSPORT_DEPENDENCY} (>= 0.1)")
elseif (RPM_PACKAGE)
    set(TYPE "RPM")
    set(GENERATOR "RPM")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}")
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    set(CPACK_${TYPE}_PACKAGE_AUTOREQ ON)
    set(CPACK_${TYPE}_PACKAGE_REQUIRES "boost-system >= 1.53, boost-regex >= 1.53, boost-filesystem >= 1.53, lib${TRANSPORT_DEPENDENCY} >= 1.0")
else ()
    return()
endif ()

# Install also configuration files
set (HTTP_SERVER_SERVICE ${PROJECT_SOURCE_DIR}/config/${PREFIX}http-server.service)
install(FILES ${HTTP_SERVER_SERVICE} DESTINATION /lib/systemd/system RENAME ${PREFIX}http-server.service)

set(CPACK_GENERATOR ${GENERATOR})
set(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
set(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
set(CPACK_${TYPE}_PACKAGE_RELEASE 1)
set(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Implementation of a HTTP server able to serve client requests using both TCP and ICN as transport protocol.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Http-server")

include(CPack)
