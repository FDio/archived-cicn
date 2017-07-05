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
project(Libicnet)
set(CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(CCNX_API icnet/ccnx)
set(CP_API icnet/transport)
set(ERRORS icnet/errors)
set(UTILS icnet/utils)
set(HTTP icnet/http)
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
        ${PROJECT_SOURCE_DIR}/${ERRORS}
        ${PROJECT_SOURCE_DIR}/${UTILS}
        ${PROJECT_SOURCE_DIR}/${HTTP}
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_VERBOSE_MAKEFILE on)

if(COMPILE_FOR_IOS)
  set(OPENSSL_ROOT_DIR $ENV{CCNX_DEPENDENCIES})
  find_host_package ( OpenSSL REQUIRED )

  set(BOOST_ROOT $ENV{CCNX_DEPENDENCIES})
  find_host_package(Boost 1.53.0 COMPONENTS system REQUIRED)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})

  find_host_package(LongBow REQUIRED)
  include_directories(${LONGBOW_INCLUDE_DIRS})

  find_host_package(Libparc REQUIRED)
  include_directories(${LIBPARC_INCLUDE_DIRS})

  find_host_package(CCNX_Common REQUIRED)
  include_directories(${CCNX_COMMON_INCLUDE_DIRS})

  find_host_package(CCNX_Transport_Rta REQUIRED)
  include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

  find_host_package(CCNX_Portal REQUIRED)
  include_directories(${CCNX_PORTAL_INCLUDE_DIRS})
else ()
  find_package(LongBow REQUIRED)
  include_directories(${LONGBOW_INCLUDE_DIRS})

  find_package(Libparc REQUIRED)
  include_directories(${LIBPARC_INCLUDE_DIRS})

  find_package(CCNX_Common REQUIRED)
  include_directories(${CCNX_COMMON_INCLUDE_DIRS})

  find_package(CCNX_Transport_Rta REQUIRED)
  include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

  find_package(CCNX_Portal REQUIRED)
  include_directories(${CCNX_PORTAL_INCLUDE_DIRS})

  find_package(Threads REQUIRED)
  include_directories(${CMAKE_THREADS_INCLUDE_DIRS})

  find_package(Boost 1.53.0 COMPONENTS system REQUIRED)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
endif()

if(ANDROID_API)
  include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/include)
  include_directories(${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include)
  set(ANDROID_LIBRARIES "${ANDROID_NDK}/platforms/android-24/arch-arm/usr/lib/libz.a" "${ANDROID_NDK}/platforms/android-24/arch-arm/usr/lib/liblog.so" "$ENV{NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/$ENV{ABI}/libgnustl_shared.so")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ANDROID_C_FLAGS} -Wall")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
  if (NOT COMPILE_FOR_IOS)
    if (BUILD_APPS)
      set(SUBFOLDERS ${APPS})
    endif (BUILD_APPS)
  endif()
endif (ANDROID_API)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${CMAKE_CXX_FLAGS} -fpermissive")

set(LIBRARIES
        ${CCNX_PORTAL_LIBRARIES}
        ${CCNX_TRANSPORT_RTA_LIBRARIES}
        ${CCNX_COMMON_LIBRARIES}
        ${LIBPARC_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${LONGBOW_LIBRARIES})

set(SUBFOLDERS ${SUBFOLDERS} icnet)

subdirs(${SUBFOLDERS})

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
string(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VENDOR ${VENDOR})
set(CPACK_PACKAGE_CONTACT ${CONTACT})
set(CPACK_COMPONENTS_ALL library headers documentation)

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

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_NAME "${PACKAGE_NAME}")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_NAME "${PACKAGE_NAME}-dev")
    set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_NAME "${PACKAGE_NAME}-doc")

    set(CPACK_${TYPE}_LIBRARY_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")
    set(CPACK_${TYPE}_HEADERS_FILE_NAME "${PACKAGE_NAME}-dev_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")
    set(CPACK_${TYPE}_DOCUMENTATION_FILE_NAME "${PACKAGE_NAME}-doc_${PACKAGE_VERSION}_${ARCHITECTURE}.deb")

    set(CPACK_DEBIAN_LIBRARY_PACKAGE_SHLIBDEPS ON)

    # No boost deps here. Let shlibdeps discover it.
    set(CPACK_${TYPE}_LIBRARY_PACKAGE_DEPENDS "longbow (>= 1.0), libparc (>= 1.0), libccnx-common (>= 1.0), libccnx-transport-rta (>= 1.0), libccnx-portal (>= 1.0)")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_DEPENDS "libccnx-common-dev (>= 1.0), libicnet (>= 1.0), libccnx-portal-dev (>= 1.0), libccnx-transport-rta-dev (>=1.0), libboost-system-dev (>= 1.53), libparc-dev (>= 1.0), longbow-dev (>= 1.0), libboost-system-dev (>= 1.53)")
    set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_DEPENDS "")
elseif(RPM_PACKAGE)
    set(TYPE "RPM")
    set(GENERATOR "RPM")

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_NAME "${PACKAGE_NAME}")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_NAME "${PACKAGE_NAME}-devel")
    set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_NAME "${PACKAGE_NAME}-doc")

    set(CPACK_${TYPE}_LIBRARY_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")
    set(CPACK_${TYPE}_HEADERS_FILE_NAME "${PACKAGE_NAME}-devel-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")
    set(CPACK_${TYPE}_DOCUMENTATION_FILE_NAME "${PACKAGE_NAME}-doc-${PACKAGE_VERSION}.${ARCHITECTURE}.rpm")

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_AUTOREQ ON)

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_REQUIRES "longbow >= 1.0, libparc >= 1.0, libccnx-common >= 1.0, libccnx-transport-rta >= 1.0, libccnx-portal >= 1.0, boost-system >= 1.53")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_REQUIRES "libicnet >= 1.0, libccnx-common-devel >= 1.0, libccnx-portal-devel >= 1.0, libccnx-transport-rta-devel >= 1.0, libparc-devel >= 1.0, longbow-devel >= 1.0, boost-devel >= 1.53")
    set(CPACK_${TYPE}_DOCUMENTATION_PACKAGE_REQUIRES "")

    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
else()
    return()
endif()

set(CPACK_GENERATOR ${GENERATOR})
set(CPACK_${GENERATOR}_COMPONENT_INSTALL ON)
set(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
set(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
set(CPACK_${TYPE}_PACKAGE_RELEASE 1)
set(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "This library is designed to provide a transport layer for applications willing to communicate using an ICN protocol stack.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Libicnet")

include(CPack)
