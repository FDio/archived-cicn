# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find LIBWEBSOCKETPP
# Find the native WEBSOCKETPP headers and libraries.
#
# WEBSOCKETPP_INCLUDE_DIRS	- where to find sqlite3.h, etc.
# WEBSOCKETPP_FOUND	- True if LIBWEBSOCKETPP found.

# Look for the header file.
FIND_PATH(WEBSOCKETPP_INCLUDE_DIR websocketpp)

# Handle the QUIETLY and REQUIRED arguments and set WEBSOCKETPP_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WEBSOCKETPP DEFAULT_MSG WEBSOCKETPP_INCLUDE_DIR)

# Copy the results to the output variables.
IF(WEBSOCKETPP_FOUND)
	SET(WEBSOCKETPP_INCLUDE_DIRS ${WEBSOCKETPP_INCLUDE_DIR})
ELSE(WEBSOCKETPP_FOUND)
	SET(WEBSOCKETPP_INCLUDE_DIRS)
ENDIF(WEBSOCKETPP_FOUND)

MARK_AS_ADVANCED(WEBSOCKETPP_INCLUDE_DIRS)
