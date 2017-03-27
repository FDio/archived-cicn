cmake_minimum_required(VERSION 3.2)
project(LongBow)

include( CTest )

set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/cmake/get_version.sh ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE RELEASE_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND date -u +%Y-%m-%dT%H:%M:%SZ
  OUTPUT_VARIABLE ISO_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

MESSAGE( STATUS "Configuring version ${RELEASE_VERSION}" )

add_definitions("-DRELEASE_VERSION=\"${RELEASE_VERSION}\"")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")

#set(CMAKE_C_FLAGS_NOPANTS "${CMAKE_C_FLAGS_NOPANTS} -O3 -DNDEBUG -DLongBow_DISABLE_ASSERTIONS")


find_package( Doxygen )
find_package( Uncrustify )
find_package( PythonInterp )

set( DEPENDENCY_DIR $ENV{CCNX_DEPENDENCIES} )
if( NOT DEPENDENCY_DIR )
  set( DEPENDENCY_DIR /usr )
endif()

set( PYVERSION ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} )

set( INSTALL_BASE_PYTHON_DIR ${CMAKE_INSTALL_PREFIX}/lib/python${PYVERSION}/site-packages )
set( INSTALL_PYTHON_DIR ${INSTALL_BASE_PYTHON_DIR}/longbow )
set( DEPENDENCY_PYTHON_DIR ${DEPENDENCY_DIR}/etc/site-packages )
set( VERSION ${RELEASE_VERSION} )
set( DATE ${ISO_DATE} )
set( UNCRUSTIFY_BIN ${UNCRUSTIFY_BIN} )
set( UNCRUSTIFY_CONFIG ${CMAKE_INSTALL_PREFIX}/etc/parc_uncrustify.cfg )

message( "-- Install Python Dir:  ${INSTALL_PYTHON_DIR}")
message( "-- Dependency Python Dir:  ${DEPENDENCY_DIR}/etc/site-packages")
include_directories(${PROJECT_BINARY_DIR}/src/LongBow ${PROJECT_SOURCE_DIR}/src)

if(ANDROID_API)
  message("############ Detected cross compile for ${CMAKE_SYSTEM_NAME}")
  message("############ This build will not include doxygen, tools, or tests")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ANDROID_C_FLAGS} -Wall")

  macro(AddTest testFile)
    message("Android build: Skipping test ${ARGV0}")
  endmacro(AddTest)

  add_subdirectory(src)
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG -DLongBow_DISABLE_ASSERTIONS -D_GNU_SOURCE")
  macro(AddTest testFile)
    add_executable(${ARGV0} ${ARGV0}.c)
    target_link_libraries(${ARGV0} ${PARC_BIN_LIBRARIES})
    add_test(${ARGV0} ${ARGV0})
    set_target_properties(${ARGV0} PROPERTIES FOLDER Test)
  endmacro(AddTest)

  add_subdirectory(src)
  add_subdirectory(documentation)

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
  elseif(RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
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
  SET(CPACK_${TYPE}_PACKAGE_REQUIRES "python >= 2.7.0")
  SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Runtime and testing framework for C applications.")

  INCLUDE(CPack)

endif()