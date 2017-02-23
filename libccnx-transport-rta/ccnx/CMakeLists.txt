configure_file(config.h.in config.h @ONLY)

add_subdirectory(api)
add_subdirectory(transport)
