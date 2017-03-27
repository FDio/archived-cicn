cmake_minimum_required(VERSION 3.2)
project(Libparc)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if( UNIX )
  link_libraries(m)
endif( UNIX )

include( CTest )
include( version )
include( detectCacheSize )

if(ANDROID_API)
    message("############ Detected cross compile for $ENV{CMAKE_SYSTEM_NAME}")
    message("############ This build will not include doxygen, tools, or tests")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ANDROID_C_FLAGS} -Wall")
else()
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#    set(CMAKE_C_FLAGS_NOPANTS "${CMAKE_C_FLAGS_NOPANTS} -O3 -DNDEBUG -DPARCLibrary_DISABLE_VALIDATION")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG -DPARCLibrary_DISABLE_VALIDATION")
    find_package( Doxygen )
endif()

include_directories($ENV{CCNX_DEPENDENCIES}/include)
set(OPENSSL_ROOT_DIR $ENV{CCNX_DEPENDENCIES})

include_directories(${PROJECT_BINARY_DIR}/parc ${PROJECT_SOURCE_DIR})

find_package( LongBow REQUIRED )
include_directories(${LONGBOW_INCLUDE_DIRS})

find_package( LibEvent REQUIRED )
include_directories(${LIBEVENT_INCLUDE_DIRS})

find_package ( Threads REQUIRED )

find_package ( OpenSSL REQUIRED )


set(PARC_BIN_LIBRARIES
  parc
  ${LONGBOW_LIBRARIES}
  ${LIBEVENT_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

add_custom_target(${PROJECT_NAME}_cleanup_profiling_data
        "find" "." "-name" "*.gcda" "-delete"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Cleanup previous profiling data.")
if (ANDROID_API)
    macro(AddTest testFile)
        message("Android build: Skipping test ${ARGV0}")
    endmacro(AddTest)
else()
    macro(AddTest testFile)
     add_executable(${ARGV0} ${ARGV0}.c)
     target_link_libraries(${ARGV0} ${PARC_BIN_LIBRARIES})
     add_test(NAME ${ARGV0} COMMAND ${ARGV0})
     set_target_properties(${ARGV0} PROPERTIES FOLDER Test)
     add_dependencies(${ARGV0} ${PROJECT_NAME}_cleanup_profiling_data)
   endmacro(AddTest)
endif()

add_subdirectory(parc)
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
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent-2.0-5, libssl1.0.0")
elseif(RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent >= 2.0, openssl >= 1.0")
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
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "The PARC Library is a C runtime providing an array of features and capabilities for C programs and programmers.")

INCLUDE(CPack)