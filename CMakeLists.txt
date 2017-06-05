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
project(Viper)

set(CMAKE_VERBOSE_MAKEFILE true)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qmake_build)

option(ICNET "Link against Libicnet." ON)
option(HICNET "Link against Libhicnet." OFF)
if (HICNET)
    set (TRANSPORT_LIBRARY HICNET)
    set(ICNET OFF)

else()
    set (TRANSPORT_LIBRARY ICNET)
endif()
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qmake_build/viper_
                  COMMAND /opt/qt57/bin/qmake ${CMAKE_SOURCE_DIR}/viper.pro "TRANSPORT_LIBRARY = ${TRANSPORT_LIBRARY}"
                  COMMAND make
                  COMMAND mv viper viper_
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qmake_build
                  VERBATIM)

add_custom_target(viper ALL
                  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/qmake_build/viper_ ./viper
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}//qmake_build/viper_)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/viper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Generate DEB / RPM packages

option(DEB_PACKAGE "Create deb package" OFF)
option(RPM_PACKAGE "Create deb package" OFF)

set(VENDOR "Cisco Systems" CACHE STRING "Vendor")
set(CONTACT "msardara@cisco.com" CACHE STRING "Contact")
set(DISTRIBUTION "xenial" CACHE STRING "Distribution")
set(ARCHITECTURE "amd64" CACHE STRING "Architecture")
set(PACKAGE_MAINTAINER "Mauro Sardara (msardara@cisco.com)" CACHE STRING "Maintainer")
set(BUILD_NUMBER "1" CACHE STRING "Build Number")
string(TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_VENDOR ${VENDOR})
set(CPACK_PACKAGE_CONTACT ${CONTACT})

# Get the version
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/version
                OUTPUT_VARIABLE PACKAGE_VERSION)
string(STRIP ${PACKAGE_VERSION} PACKAGE_VERSION)

if(DEB_PACKAGE)
    set(TYPE "DEBIAN")
    set(GENERATOR "DEB")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}")
    set(CPACK_${TYPE}_PACKAGE_SHLIBDEPS ON)
    set(CPACK_${TYPE}_PACKAGE_DEPENDS "qt57base, qt57svg, qt57charts-no-lgpl, qt57multimedia, libqtav1, libicnet, libavcodec-ffmpeg56, libavformat-ffmpeg56, libswscale-ffmpeg3, libavresample-ffmpeg2, libdash, libqml-module-qtav, qt57quickcontrols, qt57quickcontrols2")
    set(CPACK_${TYPE}_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/scripts/postinst;")
elseif(RPM_PACKAGE)
    set(TYPE "RPM")
    set(GENERATOR "RPM")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}.${ARCHITECTURE}")
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    set(CPACK_${TYPE}_PACKAGE_AUTOREQ ON)
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
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Video player able to test adaptation video streaming using the ICN protocol stack.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Viper")

include(CPack)
