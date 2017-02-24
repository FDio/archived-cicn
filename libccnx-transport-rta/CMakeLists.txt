cmake_minimum_required(VERSION 3.2)
project(Libccnx-transport)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG -DCCNxTransport_DISABLE_VALIDATION -DLIBRTA_DISABLE_VALIDATION -DTransportLibrary_DISABLE_VALIDATION")
endif()

#set(CMAKE_C_FLAGS_NOPANTS "${CMAKE_C_FLAGS_NOPANTS} -O3 -DNDEBUG -DCCNxTransport_DISABLE_VALIDATION -DLIBRTA_DISABLE_VALIDATION -DTransportLibrary_DISABLE_VALIDATION")

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/ccnx/transport/transport_rta ${PROJECT_BINARY_DIR}/ccnx)
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

find_package ( Threads REQUIRED )

find_package ( OpenSSL REQUIRED )

find_package( Doxygen )

if (ANDROID_API)
    macro(AddTest testFile)
        message("Android build: Skipping test ${ARGV0}")
    endmacro(AddTest)
else()
   macro(AddTest testFile)
     add_executable(${ARGV0} ${ARGV0}.c)
     target_link_libraries(${ARGV0} ${LONGBOW_LIBRARIES})
     target_link_libraries(${ARGV0} ${LIBEVENT_LIBRARIES})
     target_link_libraries(${ARGV0} ${OPENSSL_LIBRARIES})
     target_link_libraries(${ARGV0} ${CMAKE_THREAD_LIBS_INIT})
     target_link_libraries(${ARGV0} ccnx_transport_rta)
     target_link_libraries(${ARGV0} ccnx_api_control)
     target_link_libraries(${ARGV0} ccnx_api_notify)
     target_link_libraries(${ARGV0} ${CCNX_COMMON_LIBRARIES})
     target_link_libraries(${ARGV0} ${LIBPARC_LIBRARIES})
     add_test(${ARGV0} ${ARGV0})
     set_target_properties(${ARGV0} PROPERTIES FOLDER Test)  
   endmacro(AddTest)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	message( "-- Set \"-undefined dynamic_lookup\" for shared libraries")
endif()

add_subdirectory(ccnx)
