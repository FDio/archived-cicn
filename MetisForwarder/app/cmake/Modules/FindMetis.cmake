########################################
#
# Find the LibMetis libraries and includes
# This module sets:
# LIBMETIS_FOUND: True if Libmetis was found
# LIBMETIS_LIBRARY: The Libmetis library
# LIBMETIS_LIBRARIES: The Libmetis library and dependencies
# LIBMETIS_INCLUDE_DIR: The Libmetis include dir
#

set(METIS_SEARCH_PATH_LIST

${LIBMETISHOME}
$ENV{LIBMETISHOME}
$ENV{CCNX_HOME} 
$ENV{FOUNDATION_HOME} 
/usr/local/
/usr/local/ccnx
/usr/local/ccn
/usr/local
/opt
/usr
)



find_path(METIS_INCLUDE_DIR ccnx/forwarder/metis/metis_About.h
HINTS ${METIS_SEARCH_PATH_LIST}
PATH_SUFFIXES include
DOC "Find the libmetis includes")

find_library(LIBMETIS_LIBRARY NAMES metis
HINTS ${LIBMETIS_SEARCH_PATH_LIST}
PATH_SUFFIXES lib
DOC "Find the libmetis libraries")

set(LIBMETIS_LIBRARIES ${LIBMETIS_LIBRARY})
set(LIBMETIS_INCLUDE_DIRS ${LIBMETIS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libmetis DEFAULT_MSG LIBMETIS_LIBRARY LIBMETIS_INCLUDE_DIR)
