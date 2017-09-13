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
project(cicn-plugin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Dependencies

find_package(Vpp REQUIRED)

set(CICN_PLUGIN_SOURCE_FILES
    cicn/cicn.c
    cicn/cicn_face.c
    cicn/cicn_fib.c
    cicn/cicn_hashtb.c
    cicn/cicn_hello.c
    cicn/cicn_mgmt.c
    cicn/cicn_parser.c
    cicn/cicn_pcs.c
    cicn/cicn_pg.c
    cicn/node.c)

set(CICN_PLUGIN_HEADER_FILES
    cicn/cicn_all_api_h.h
    cicn/cicn_api_handler.h
    cicn/cicn_fib.h
    cicn/cicn_hashtb.h
    cicn/cicn_hello_inlines.h
    cicn/cicn_mgmt.h
    cicn/cicn_params.h
    cicn/cicn_pcs.h
    cicn/cicn_std.h
    cicn/cicn_api.h
    cicn/cicn_face.h
    cicn/cicn.h
    cicn/cicn_hello.h
    cicn/cicn_infra.h
    cicn/cicn_msg_enum.h
    cicn/cicn_parser.h
    cicn/cicn_siphash.h
    cicn/cicn_types.h)

if(NOT NO_UNIT_TEST)
    list(APPEND CICN_PLUGIN_SOURCE_FILES
            cicn/test/test_cicn.c
            cicn/test/test_cicn_hash.c
            cicn/test/test_cicn_hash_cdata.c)
    list(APPEND CICN_PLUGIN_HEADER_FILES
            cicn/test/test_cicn.h
            cicn/test/test_cicn_hash.h)
endif(NOT NO_UNIT_TEST)

set(CICN_API_TEST_SOURCE_FILES
        cicn/cicn_api_test.c)

set(CICN_API_TEST_HEADER_FILES
        cicn/cicn_api.h)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Release")
endif (NOT CMAKE_BUILD_TYPE)


SET(CICN_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "cicn_install_prefix")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall  -march=native -O3")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -march=native -O0 -g")
  add_definitions(-DCLIB_DEBUG -fPIC -fstack-protector-all)
endif()

execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/cicn)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cicn/cicn.api.h
        COMMAND
        ${CMAKE_C_COMPILER} $ENV{CPPFLAGS} -E -P -x c-header ${CMAKE_CURRENT_SOURCE_DIR}/cicn/cicn.api |
        vppapigen --input - --output ${CMAKE_CURRENT_BINARY_DIR}/cicn/cicn.api.h
        --show-name ${CMAKE_CURRENT_BINARY_DIR}/cicn/cicn.api.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cicn/cicn.api)

include_directories(SYSTEM)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(./)
include_directories(cicn/test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -O3")
add_library(cicn_api_test_plugin SHARED
        ${CICN_API_TEST_SOURCE_FILES}
        ${CICN_API_TEST_HEADER_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/cicn/cicn.api.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCICN_VPP_PLUGIN=1")
add_library(cicn_plugin SHARED
        ${CICN_PLUGIN_SOURCE_FILES}
        ${CICN_PLUGIN_HEADER_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/cicn/cicn.api.h)

set_target_properties(cicn_plugin
        PROPERTIES
        LINKER_LANGUAGE C)
set_target_properties(cicn_api_test_plugin
        PROPERTIES
        LINKER_LANGUAGE C)

target_link_libraries(cicn_plugin)
target_link_libraries(cicn_api_test_plugin)

set(VPP_INSTALL_PLUGIN ${CICN_INSTALL_PREFIX}/vpp_plugins)
set(VPP_INSTALL_API_TEST_PLUGIN ${CICN_INSTALL_PREFIX}/vpp_api_test_plugins)

install(DIRECTORY DESTINATION ${VPP_INSTALL_PLUGIN})
install(TARGETS cicn_plugin
        DESTINATION
        ${VPP_INSTALL_PLUGIN})

install(DIRECTORY DESTINATION ${VPP_INSTALL_API_TEST_PLUGIN})
install(TARGETS cicn_api_test_plugin
        DESTINATION
        ${VPP_INSTALL_API_TEST_PLUGIN})

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

SET(VENDOR "Cisco Systems" CACHE STRING "Vendor")
SET(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
SET(DISTRIBUTION "xenial" CACHE STRING "Distribution")
SET(ARCHITECTURE "amd64" CACHE STRING "Architecture")
SET(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
SET(BUILD_NUMBER "1" CACHE STRING "Build Number")
STRING(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
SET(CPACK_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_PACKAGE_CONTACT ${CONTACT})

# Get the version
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/version
                OUTPUT_VARIABLE PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)

if (DEB_PACKAGE)
    SET(TYPE "DEBIAN")
    SET(GENERATOR "DEB")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}")
    SET(CPACK_${TYPE}_PACKAGE_DEPENDS "vpp (>= 17.04), vpp (< 17.07), vpp-lib (>= 17.04), vpp-lib (< 17.07), vpp-plugins (>= 17.04), vpp-plugins (< 17.07)")
elseif (RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "vpp >= 17.04, vpp < 17.07, vpp-lib >= 17.04, vpp-lib < 17.07, vpp-plugins >= 17.04, vpp-plugins < 17.07")
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
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "High-performance CCNx ICN forwarder as a plugin to VPP.")
SET(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Cicn-plugin")

INCLUDE(CPack)
