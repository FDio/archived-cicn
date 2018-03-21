########################################
#
# Find the Libccnx-transport libraries and includes
# This module sets:
#  CCNX_TRANSPORT_RTA_FOUND: True if Libparc was found
#  CCNX_TRANSPORT_RTA_LIBRARY:  The Libparc library
#  CCNX_TRANSPORT_RTA_LIBRARIES:  The Libparc library and dependencies
#  CCNX_TRANSPORT_RTA_INCLUDE_DIR:  The Libparc include dir
#

set(CCNX_TRANSPORT_RTA_SEARCH_PATH_LIST
  ${CCNX_TRANSPORT_RTA_HOME} 
  $ENV{CCNX_TRANSPORT_RTA_HOME} 
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

find_path(CCNX_TRANSPORT_RTA_INCLUDE_DIR ccnx/transport/librta_About.h
  HINTS ${CCNX_TRANSPORT_RTA_SEARCH_PATH_LIST}
  PATH_SUFFIXES include
  DOC "Find the Libccnx-transport-rta includes" )
	  
find_library(CCNX_TRANSPORT_RTA_LIBRARY NAMES ccnx_transport_rta
  HINTS ${CCNX_TRANSPORT_RTA_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libccnx-transport-rta libraries" )

find_library(CCNX_API_NOTIFY_LIBRARY NAMES ccnx_api_notify
  HINTS ${CCNX_TRANSPORT_RTA_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libccnx-transport-rta libraries" )

find_library(CCNX_API_CONTROL_LIBRARY NAMES ccnx_api_control
  HINTS ${CCNX_TRANSPORT_RTA_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libccnx-transport-rta libraries" )

set(CCNX_TRANSPORT_RTA_LIBRARIES ${CCNX_TRANSPORT_RTA_LIBRARY} ${CCNX_API_CONTROL_LIBRARY} ${CCNX_API_NOTIFY_LIBRARY})

set(CCNX_TRANSPORT_RTA_INCLUDE_DIRS ${CCNX_TRANSPORT_RTA_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCNX_Transport_Api  DEFAULT_MSG CCNX_TRANSPORT_RTA_LIBRARY CCNX_TRANSPORT_RTA_INCLUDE_DIR)
