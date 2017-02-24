set(PARC_PUBLICKEY_SRC 
  parc-publickey.c	
  parcPublicKey_About.c
  )

add_executable(parc-publickey ${PARC_PUBLICKEY_SRC})
target_link_libraries(parc-publickey ${PARC_BIN_LIBRARIES})
install( TARGETS parc-publickey RUNTIME DESTINATION bin )
