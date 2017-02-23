find_package(Doxygen)

if(DOXYGEN_FOUND)
  configure_file(libccnx_api_portal-stage1.doxygen.in libccnx_api_portal-stage1.doxygen @ONLY)
  configure_file(libccnx_api_portal-stage2.doxygen.in libccnx_api_portal-stage2.doxygen @ONLY)
  add_custom_target(docs
	DEPENDS doctags documentation)
  add_custom_target(doctags
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/libccnx_api_portal-stage1.doxygen
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Doctags" VERBATIM )
  add_custom_target(documentation
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/libccnx_api_portal-stage2.doxygen
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Documentation" VERBATIM )
  add_custom_target(clobber-docs
	COMMAND /bin/rm -rf ${CMAKE_CURRENT_BINARY_DIR}/libccnx_api_portal-documentation
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Removing Documentation" VERBATIM )
endif(DOXYGEN_FOUND)
