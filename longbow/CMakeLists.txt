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
endif()