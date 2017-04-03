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

set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_VERBOSE_MAKEFILE true)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qmake_build)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qmake_build/viper_
                  COMMAND /opt/qt57/bin/qmake ${CMAKE_SOURCE_DIR}/viper.pro
                  COMMAND make
                  COMMAND mv viper viper_
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qmake_build
                  VERBATIM)

add_custom_target(viper ALL
                  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/qmake_build/viper_ ./viper
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}//qmake_build/viper_)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/viper DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

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

if(DEB_PACKAGE)
    SET(TYPE "DEBIAN")
    SET(GENERATOR "DEB")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}~${DISTRIBUTION}_${ARCHITECTURE}")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "qt57base, qt57svg, qt57charts-no-lgpl, qt57multimedia, libqtav-dev, libicnet, libavcodec-dev, libavformat-dev, libswscale-dev,  libavresample-dev, libdash, libqml-module-qtav, qt57quickcontrols, qt57quickcontrols")
elseif(RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "")
else()
    RETURN()
endif()

SET(CPACK_GENERATOR ${GENERATOR})
SET(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
SET(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
SET(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
SET(CPACK_${TYPE}_PACKAGE_RELEASE 1)
SET(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Video player able to test adaptation video streaming using the ICN protocol stack.")

INCLUDE(CPack)
