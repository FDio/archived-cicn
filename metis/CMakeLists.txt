cmake_minimum_required(VERSION 3.2)

project(Metis)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_WLDR "build with WLDR" ON)
message("-- Building with WLDR: " ${BUILD_WLDR})

if( BUILD_WLDR )
    set(WLDR ON CACHE BOOL, "-- Compiling with WLDR")
endif( BUILD_WLDR )

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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
endif()
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/ccnx/forwarder/metis)

include_directories($ENV{CCNX_DEPENDENCIES}/include)
set(OPENSSL_ROOT_DIR $ENV{CCNX_DEPENDENCIES})

find_package( LongBow REQUIRED )
include_directories(${LONGBOW_INCLUDE_DIRS})

find_package( LibEvent REQUIRED )
include_directories(${LIBEVENT_INCLUDE_DIRS})

find_package( Libparc REQUIRED )
include_directories(${LIBPARC_INCLUDE_DIRS})

find_package( CCNX_Common REQUIRED )
include_directories(${CCNX_COMMON_INCLUDE_DIRS})

find_package( CCNX_Transport_Rta REQUIRED )
include_directories(${CCNX_TRANSPORT_RTA_INCLUDE_DIRS})

find_package( CCNX_Portal REQUIRED )
include_directories(${CCNX_PORTAL_INCLUDE_DIRS})

find_package ( Threads REQUIRED )

find_package ( OpenSSL REQUIRED )

find_package( Doxygen )

set(METIS_LINK_LIBRARIES
  metis
  ${LONGBOW_LIBRARIES}
  ${LIBEVENT_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CCNX_PORTAL_LIBRARIES}
  ${CCNX_TRANSPORT_RTA_LIBRARIES}
  ${CCNX_COMMON_LIBRARIES}
  ${LIBPARC_LIBRARIES}
  )

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

if (ANDROID_API)
    macro(AddTest testFile)
        message("Android build: Skipping test ${ARGV0}")
    endmacro(AddTest)
else()
   macro(AddTest testFile)
     add_executable(${ARGV0} ${ARGV0}.c)
     target_link_libraries(${ARGV0} ${METIS_LINK_LIBRARIES})
     add_test(${ARGV0} ${ARGV0})
     set_target_properties(${ARGV0} PROPERTIES FOLDER Test)
   endmacro(AddTest)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	message( "-- Set \"-undefined dynamic_lookup\" for shared libraries")
endif()

add_subdirectory(ccnx/forwarder/metis)

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
STRING(TOLOWER ${CMAKE_PROJECT_NAME}-forwarder PACKAGE_NAME)

SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_PACKAGE_CONTACT ${CONTACT})

if(DEB_PACKAGE)
    SET(TYPE "DEBIAN")
    SET(GENERATOR "DEB")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}~${DISTRIBUTION}_${ARCHITECTURE}")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent-2.0-5, libssl1.0.0, libparc >= 1.0, libccnx-common >= 1.0")
elseif(RPM_PACKAGE)
    SET(TYPE "RPM")
    SET(GENERATOR "RPM")
    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-${BUILD_NUMBER}.${DISTRIBUTION}.${ARCHITECTURE}")
    SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/etc" "/usr/lib/python2.7" "/usr/lib/python2.7/site-packages" "/lib/systemd/system" "/lib" "/lib/systemd")
    SET(CPACK_${TYPE}_PACKAGE_REQUIRES "longbow >= 1.0, libevent >= 2.0, openssl >= 1.0, libparc >= 1.0, libccnx-common >= 1.0")
else()
    RETURN()
endif()

# Install also configuration files
set (METIS_CONFIG ${PROJECT_SOURCE_DIR}/config/ccnx_metis.conf)
set (METIS_SERVICE ${PROJECT_SOURCE_DIR}/config/metis-forwarder.service)

install(FILES ${METIS_CONFIG} DESTINATION /etc/ccnx)
install(FILES ${METIS_SERVICE} DESTINATION /lib/systemd/system)

SET(CPACK_GENERATOR ${GENERATOR})
SET(CPACK_${TYPE}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
SET(CPACK_${TYPE}_PACKAGE_NAME ${PACKAGE_NAME})
SET(CPACK_${TYPE}_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_${TYPE}_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
SET(CPACK_${TYPE}_PACKAGE_RELEASE 1)
SET(CPACK_${TYPE}_PACKAGE_VENDOR ${VENDOR})
SET(CPACK_${TYPE}_PACKAGE_DESCRIPTION "Metis is a CCNx socket based forwarder")

INCLUDE(CPack)
