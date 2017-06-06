# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FIND_ROOT_PATH "$ENV{CCNX_HOME};${CMAKE_FIND_ROOT_PATH}")
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/metis-forwarder)

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

find_package( Metis REQUIRED)
include_directories(${Metis_INCLUDE_DIRS})

add_library( # Sets the name of the library.
             metisForwarderWrap

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/c/metisForwarderWrap.c )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
                       metisForwarderWrap

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} ${LIBMETIS_LIBRARIES} ${CCNX_PORTAL_LIBRARIES} ${CCNX_TRANSPORT_RTA_LIBRARIES}  ${CCNX_COMMON_LIBRARIES} ${LIBPARC_LIBRARIES} ${LONGBOW_LIBRARIES} ${LIBEVENT_LIBRARIES} ${OPENSSL_LIBRARIES})