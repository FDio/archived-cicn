########################################
#
# Find the Libparc libraries and includes
# This module sets:
#  LIBPARC_FOUND: True if Libparc was found
#  LIBPARC_LIBRARY:  The Libparc library
#  LIBPARC_LIBRARIES:  The Libparc library and dependencies
#  LIBPARC_INCLUDE_DIR:  The Libparc include dir
#

set(LIBPARC_SEARCH_PATH_LIST
  ${LIBPARC_HOME} 
  $ENV{LIBPARC_HOME} 
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

find_path(LIBPARC_INCLUDE_DIR parc/libparc_About.h
  HINTS ${LIBPARC_SEARCH_PATH_LIST}
  PATH_SUFFIXES include
  DOC "Find the Libparc includes" )
	  
find_library(LIBPARC_LIBRARY NAMES parc
  HINTS ${LIBPARC_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the Libparc libraries" )

set(LIBPARC_LIBRARIES ${LIBPARC_LIBRARY})
set(LIBPARC_INCLUDE_DIRS ${LIBPARC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libparc  DEFAULT_MSG LIBPARC_LIBRARY LIBPARC_INCLUDE_DIR)
