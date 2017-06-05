if(NOT COMPILE_FOR_IOS)
  add_executable(metis_daemon metis_daemon.c)
  target_link_libraries(metis_daemon ${METIS_LINK_LIBRARIES})

  install(TARGETS metis_daemon RUNTIME DESTINATION bin)
endif()
