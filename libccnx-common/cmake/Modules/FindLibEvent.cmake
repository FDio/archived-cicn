########################################
#
# Find the LibEvent libraries and includes
# This module sets:
#  LIBEVENT_FOUND: True if LibEvent was found
#  LIBEVENT_LIBRARY:  The LibEvent library
#  LIBEVENT_LIBRARIES:  The LibEvent library and dependencies
#  LIBEVENT_INCLUDE_DIR:  The LibEvent include dir
#
# This module will look for the libraries in various locations
# See the LIBEVENT_SEARCH_PATH_LIST for a full list.
#
# The caller can hint at locations using the following variables:
#
# LIBEVENT_HOME (passed as -D to cmake)
# CCNX_DEPENDENCIES (in environment)
# LIBEVENT_HOME (in environment)
# CCNX_HOME (in environment)
#

set(LIBEVENT_SEARCH_PATH_LIST
  ${LIBEVENT_HOME} 
  $ENV{CCNX_DEPENDENCIES} 
  $ENV{LIBEVENT_HOME} 
  $ENV{CCNX_HOME} 
  /usr/local/ccnx 
  /usr/local/ccn 
  /usr/local 
  /opt
  /usr 
  )

find_path(LIBEVENT_INCLUDE_DIR event2/event.h
  HINTS ${LIBEVENT_SEARCH_PATH_LIST}
  PATH_SUFFIXES include 
  DOC "Find the LibEvent includes" )
	  
find_library(LIBEVENT_LIBRARY NAMES event
  HINTS ${LIBEVENT_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib 
  DOC "Find the LibEvent libraries" )

set(LIBEVENT_LIBRARIES ${LIBEVENT_LIBRARY})
set(LIBEVENT_INCLUDE_DIRS ${LIBEVENT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibEvent  DEFAULT_MSG LIBEVENT_LIBRARY LIBEVENT_INCLUDE_DIR)
