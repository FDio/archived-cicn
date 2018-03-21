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
set(CMAKE_FIND_ROOT_PATH "$ENV{DISTILLERY_ROOT_DIR}/usr_${CMAKE_SYSTEM_PROCESSOR};${CMAKE_FIND_ROOT_PATH}")

find_package(Boost 1.53.0 COMPONENTS regex system filesystem REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

find_package ( OpenSSL REQUIRED )

find_package( LibEvent REQUIRED )
include_directories(${LIBEVENT_INCLUDE_DIRS})

find_package( LongBow REQUIRED )
include_directories(${LONGBOW_INCLUDE_DIRS})

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

find_package(Jsoncpp REQUIRED)
include_directories(${JSONCPP_INCLUDE_DIRS})


find_package(Libicnet REQUIRED)
include_directories(${LIBICNET_INCLUDE_DIRS})

find_package(CURL REQUIRED)

find_package( HttpServer REQUIRED)
include_directories(${LIB_HTTP_SERVER_INCLUDE_DIRS})

add_library( # Sets the name of the library.
             httpServerWrap

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/httpServerWrap.cpp )

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
                       httpServerWrap

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} ${LIB_HTTP_SERVER_LIBRARIES} ${CURL_LIBRARIES}  ${ANDROID_LIBRARIES} ${LIBICNET_LIBRARIES} ${LIBZF_LOG_LIBRARIES} ${Boost_LIBRARIES} ${JSONCPP_LIBRARIES} ${CCNX_PORTAL_LIBRARIES}  ${CCNX_TRANSPORT_RTA_LIBRARIES}  ${CCNX_COMMON_LIBRARIES} ${LIBPARC_LIBRARIES} ${LONGBOW_LIBRARIES} ${LIBEVENT_LIBRARIES} ${OPENSSL_LIBRARIES})