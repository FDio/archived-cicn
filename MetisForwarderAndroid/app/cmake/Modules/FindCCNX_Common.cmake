########################################
#
# Find the Libparc libraries and includes
# This module sets:
#  CCNX_COMMON_FOUND: True if Libparc was found
#  CCNX_COMMON_LIBRARY:  The Libparc library
#  CCNX_COMMON_LIBRARIES:  The Libparc library and dependencies
#  CCNX_COMMON_INCLUDE_DIR:  The Libparc include dir
#

set(CCNX_COMMON_SEARCH_PATH_LIST
  ${CCNX_COMMON_HOME} 
  $ENV{CCNX_COMMON_HOME} 
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

find_path(CCNX_COMMON_INCLUDE_DIR ccnx/common/libccnxCommon_About.h
  HINTS ${CCNX_COMMON_SEARCH_PATH_LIST}
  PATH_SUFFIXES include
  DOC "Find the Libccnx-common includes" )
	  
find_library(CCNX_COMMON_LIBRARY NAMES ccnx_common
  HINTS ${CCNX_COMMON_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libccnx-common libraries" )

set(CCNX_COMMON_LIBRARIES ${CCNX_COMMON_LIBRARY})
set(CCNX_COMMON_INCLUDE_DIRS ${CCNX_COMMON_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCNX_Common  DEFAULT_MSG CCNX_COMMON_LIBRARY CCNX_COMMON_INCLUDE_DIR)
