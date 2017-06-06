########################################
#
# Find the Libccnx-transport libraries and includes
# This module sets:
#  CCNX_PORTAL_FOUND: True if Libparc was found
#  CCNX_PORTAL_LIBRARY:  The Libparc library
#  CCNX_PORTAL_LIBRARIES:  The Libparc library and dependencies
#  CCNX_PORTAL_INCLUDE_DIR:  The Libparc include dir
#

set(CCNX_PORTAL_SEARCH_PATH_LIST
  ${CCNX_PORTAL_HOME} 
  $ENV{CCNX_PORTAL_HOME} 
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

find_path(CCNX_PORTAL_INCLUDE_DIR ccnx/api/ccnx_Portal/ccnxPortal_About.h
  HINTS ${CCNX_PORTAL_SEARCH_PATH_LIST}
  PATH_SUFFIXES include
  DOC "Find the Libccnx-portal includes" )
	  
find_library(CCNX_PORTAL_LIBRARY NAMES ccnx_api_portal
  HINTS ${CCNX_PORTAL_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libccnx-portal libraries" )

set(CCNX_PORTAL_LIBRARIES ${CCNX_PORTAL_LIBRARY})

set(CCNX_PORTAL_INCLUDE_DIRS ${CCNX_PORTAL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCNX_Portal  DEFAULT_MSG CCNX_PORTAL_LIBRARY CCNX_PORTAL_INCLUDE_DIR)
