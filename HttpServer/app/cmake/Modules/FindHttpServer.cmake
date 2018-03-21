########################################
#
# Find the HttpServer libraries and includes
# This module sets:
#  LIB_HTTP_SERVER_FOUND: True if lib_http_server was found
#  LIB_HTTP_SERVER_LIBRARY:  The lib_http_server library
#  LIB_HTTP_SERVER_LIBRARIES:  The lib_http_server library and dependencies
#  LIB_HTTP_SERVER_INCLUDE_DIR:  The lib_http_server include dir
#

set(LIBICN_HTTP_SERVER_SEARCH_PATH_LIST
        ${HTTP_SERVER_HOME}
        $ENV{HTTP_SERVER_HOME}
        $ENV{CCNX_HOME}
        $ENV{PARC_HOME}
        $ENV{FOUNDATION_HOME}
        /usr/local/http-server
        /usr/local/ccnx
        /usr/local/ccn
        /usr/local
        /opt
        /usr
        )

find_path(LIB_HTTP_SERVER_INCLUDE_DIR http-server/http_server.h
        HINTS ${LIBICN_HTTP_SERVER_SEARCH_PATH_LIST}
        PATH_SUFFIXES include
        DOC "Find the http-server includes")

find_library(LIB_HTTP_SERVER_LIBRARY NAMES httpserver
        HINTS ${LIB_HTTP_SERVER_SEARCH_PATH_LIST}
        PATH_SUFFIXES lib
        DOC "Find the http-server libraries")

set(LIB_HTTP_SERVER_LIBRARIES ${LIB_HTTP_SERVER_LIBRARY})
set(LIB_HTTP_SERVER_INCLUDE_DIRS ${LIB_HTTP_SERVER_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lib_http_server DEFAULT_MSG LIB_HTTP_SERVER_LIBRARY LIB_HTTP_SERVER_INCLUDE_DIR)
