#
# Get a version to pass on the command line
#
execute_process(COMMAND ${PROJECT_SOURCE_DIR}/cmake/get_version.sh ${PROJECT_SOURCE_DIR} 
  OUTPUT_VARIABLE RELEASE_VERSION 
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND date -u +%Y-%m-%dT%H:%M:%SZ
  OUTPUT_VARIABLE ISO_DATE 
  OUTPUT_STRIP_TRAILING_WHITESPACE)

MESSAGE( STATUS "Configuring version ${RELEASE_VERSION}" )

add_definitions("-DRELEASE_VERSION=\"${RELEASE_VERSION}\"")

