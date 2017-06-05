########################################
#
# Find the Libhicnet libraries and includes
# This module sets:
#  LIBHICNET_FOUND: True if Libconsumer-producer was found
#  LIBHICNETR_LIBRARY:  The Libconsumer-producer library
#  LIBHICNET_LIBRARIES:  The Libconsumer-producer library and dependencies
#  LIBHICNET_INCLUDE_DIR:  The Libconsumer-producer include dir
#

set(LIBHICNET_SEARCH_PATH_LIST
        ${LIBHICNET_HOME}
        $ENV{LIBHICNETHOME}
        $ENV{CCNX_HOME}
        $ENV{PARC_HOME}
        $ENV{FOUNDATION_HOME}
        /usr/local/parc
        /usr/local/ccnx
        /usr/local/ccn
        /usr/local
        /opt
        /usr
        )

find_path(LIBHICNET_INCLUDE_DIR hicnet/hicnet_core_common.h
        HINTS ${LIBHICNET_SEARCH_PATH_LIST}
        PATH_SUFFIXES include
        DOC "Find the libhicnet includes")

find_library(LIBHICNET_LIBRARY NAMES hicnet
        HINTS ${LIBHICNET_SEARCH_PATH_LIST}
        PATH_SUFFIXES lib
        DOC "Find the libhicnet libraries")

set(LIBHICNET_LIBRARIES ${LIBHICNET_LIBRARY})
set(LIBHICNET_INCLUDE_DIRS ${LIBHICNET_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libhicnet DEFAULT_MSG LIBHICNET_LIBRARY LIBHICNET_INCLUDE_DIR)
