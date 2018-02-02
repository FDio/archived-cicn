cmake_minimum_required(VERSION 3.2)
project(Libparc)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

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
elseif(COMPILE_FOR_IOS)
    message("############ Detected cross compile for $ENV{CMAKE_SYSTEM_NAME}")
    message("############ This build will not include doxygen, tools, or tests")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
else()
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#    set(CMAKE_C_FLAGS_NOPANTS "${CMAKE_C_FLAGS_NOPANTS} -O3 -DNDEBUG -DPARCLibrary_DISABLE_VALIDATION")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    find_package( Doxygen )
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS} -DPARCLibrary_DISABLE_VALIDATION -DLongBow_DISABLE_ASSERTIONS")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${CMAKE_C_FLAGS}")

include_directories($ENV{CCNX_DEPENDENCIES}/include)
set(OPENSSL_ROOT_DIR $ENV{CCNX_DEPENDENCIES})

include_directories(${PROJECT_BINARY_DIR}/parc ${PROJECT_SOURCE_DIR})

if(COMPILE_FOR_IOS)
  find_host_package( LongBow REQUIRED )
  include_directories(${LONGBOW_INCLUDE_DIRS})

  find_host_package( LibEvent REQUIRED )
  include_directories(${LIBEVENT_INCLUDE_DIRS})

  find_host_package ( OpenSSL REQUIRED )
else()
  find_package( LongBow REQUIRED )
  include_directories(${LONGBOW_INCLUDE_DIRS})

  find_package( LibEvent REQUIRED )
  include_directories(${LIBEVENT_INCLUDE_DIRS})

  find_package ( Threads REQUIRED )

  find_package ( OpenSSL REQUIRED )

endif()

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
elseif(COMPILE_FOR_IOS)
	macro(AddTest testFile)
        message("iOS build: Skipping test ${ARGV0}")
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

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_DEPENDS "longbow (>= 1.0)")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_DEPENDS "libparc (>= 1.0), longbow-dev (>= 1.0), libevent-dev, libssl-dev")
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

    set(CPACK_${TYPE}_LIBRARY_PACKAGE_REQUIRES "longbow >= 1.0, libevent >= 2.0, openssl >= 1.0")
    set(CPACK_${TYPE}_HEADERS_PACKAGE_REQUIRES "libparc >= 1.0, longbow-devel >= 1.0, libevent-devel, openssl-devel")
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
set(CPACK_${TYPE}_PACKAGE_DESCRIPTION "The PARC Library is a C runtime providing an array of features and capabilities for C programs and programmers.")
set(CPACK_${TYPE}_PACKAGE_HOMEPAGE "https://wiki.fd.io/view/Cframework")

include(CPack)
