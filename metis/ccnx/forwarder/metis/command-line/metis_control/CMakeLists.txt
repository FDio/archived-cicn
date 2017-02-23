add_executable(metis_control metisControl_main.c)
target_link_libraries(metis_control ${METIS_LINK_LIBRARIES})

install(TARGETS metis_control RUNTIME DESTINATION bin)
