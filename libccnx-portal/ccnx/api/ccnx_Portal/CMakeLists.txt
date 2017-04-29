# Define a few configuration variables that we want accessible in the software

configure_file(config.h.in config.h @ONLY)

set(CCNX_API_PORTAL_HEADERS
	ccnx_Portal.h
	ccnx_PortalFactory.h
	ccnx_PortalAttributes.h
	ccnx_PortalStack.h
	ccnx_PortalRTA.h
	ccnx_PortalAPI.h
	ccnx_PortalAnchor.h
	ccnxPortal_About.h
	)

set(CCNX_API_PORTAL_SOURCE_FILES
   ${CCNX_API_PORTAL_HEADERS}
    ccnx_Portal.c
    ccnx_PortalFactory.c
    ccnx_PortalAttributes.c
    ccnx_PortalStack.c
    ccnx_PortalRTA.c
    ccnx_PortalAPI.c
    ccnx_PortalAnchor.c
    ccnxPortal_About.c
    )

source_group(Sources FILES ${CCNX_API_PORTAL_SOURCE_FILES})
source_group(Sources FILES ${CCNX_API_PORTAL_HEADERS})

add_library(ccnx_api_portal         STATIC ${CCNX_API_PORTAL_SOURCE_FILES} ${CCNX_API_PORTAL_HEADER_FILES})
add_library(ccnx_api_portal.shared  SHARED ${CCNX_API_PORTAL_SOURCE_FILES})
set_target_properties(ccnx_api_portal.shared PROPERTIES
  C_STANDARD 99
  SOVERSION 1
  VERSION 1.0
  OUTPUT_NAME ccnx_api_portal )

set(libccnx_api_portal_libraries
  ccnx_api_portal
  ccnx_api_portal.shared
  )

foreach(lib ${libccnx_api_portal_libraries})
  install(TARGETS ${lib} COMPONENT library LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${CCNX_API_PORTAL_HEADERS} DESTINATION include/ccnx/api/ccnx_Portal COMPONENT headers)

add_subdirectory(test)
add_subdirectory(command-line)
