set(CCNX_CLIENT_SRC
  ccnx-client.c
  ccnxPortalClient_About.c
  )

set(CCNX_SERVER_SRC
  ccnx-server.c
  ccnxPortalServer_About.c
  )



add_executable(ccnx-client ${CCNX_CLIENT_SRC})
target_link_libraries(ccnx-client ${CCNX_LINK_LIBRARIES})

add_executable(ccnx-server ${CCNX_SERVER_SRC})
target_link_libraries(ccnx-server ${CCNX_LINK_LIBRARIES})

install(TARGETS ccnx-client ccnx-server COMPONENT library RUNTIME DESTINATION bin )
