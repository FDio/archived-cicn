cmake_minimum_required(VERSION 3.2)
project(LongBow)

include( CTest )

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
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -D_GNU_SOURCE")
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

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_DEPENDS "python (>= 2.7.0)")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_DEPENDS "longbow (>= 1.0)")
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

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_REQUIRES "python >= 2.7.0")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_REQUIRES "longbow >= 1.0")
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
  set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Runtime and testing framework for C applications.")
  set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Cframework")

  include(CPack)

endif()