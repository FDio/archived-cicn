# Define a few configuration variables that we want accessible in the software

set(CCNX_API_NOTIFY_HEADERS
	ccnxNotifyAPI_About.h 
	notify_Status.h       
	notify_Timer.h
)	

set(CCNX_API_NOTIFY_SOURCE_FILES 
	ccnxNotifyAPI_About.c 
	notify_Status.c
)


add_library(ccnx_api_notify         STATIC ${CCNX_API_NOTIFY_SOURCE_FILES} ${CCNX_API_NOTIFY_HEADERS})
add_library(ccnx_api_notify.shared  SHARED ${CCNX_API_NOTIFY_SOURCE_FILES})

source_group(Sources FILES ${CCNX_API_NOTIFY_SOURCE_FILES})
source_group(Sources FILES ${CCNX_API_NOTIFY_HEADERS})

set_target_properties(ccnx_api_notify.shared PROPERTIES 
  C_STANDARD 99
  SOVERSION 1 
  VERSION 1.0
  OUTPUT_NAME ccnx_api_notify )

set(libccnx_api_notify_libraries
  ccnx_api_notify
  ccnx_api_notify.shared
  )

foreach(lib ${libccnx_api_notify_libraries})
  install(TARGETS ${lib} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${CCNX_API_NOTIFY_HEADERS} DESTINATION include/ccnx/api/notify )
	
#add_subdirectory(test)
