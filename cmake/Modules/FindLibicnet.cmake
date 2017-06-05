########################################
#
# Find the Libparc libraries and includes
# This module sets:
#  LIBICNET_FOUND: True if Libconsumer-producer was found
#  LIBICNETR_LIBRARY:  The Libconsumer-producer library
#  LIBICNET_LIBRARIES:  The Libconsumer-producer library and dependencies
#  LIBICNET_INCLUDE_DIR:  The Libconsumer-producer include dir
#

set(LIBICNET_SEARCH_PATH_LIST
        ${LIBICNET_HOME}
        $ENV{LIBICNETHOME}
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

find_path(LIBICNET_INCLUDE_DIR icnet/icnet_ccnx_common.h
        HINTS ${LIBICNET_SEARCH_PATH_LIST}
        PATH_SUFFIXES include
        DOC "Find the libicnet includes")

find_library(LIBICNET_LIBRARY NAMES icnet
        HINTS ${LIBICNET_SEARCH_PATH_LIST}
        PATH_SUFFIXES lib
        DOC "Find the libicnet libraries")

set(LIBICNET_LIBRARIES ${LIBICNET_LIBRARY})
set(LIBICNET_INCLUDE_DIRS ${LIBICNET_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libicnet DEFAULT_MSG LIBICNET_LIBRARY LIBICNET_INCLUDE_DIR)
