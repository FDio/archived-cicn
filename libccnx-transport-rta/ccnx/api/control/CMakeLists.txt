# Define a few configuration variables that we want accessible in the software

set(CCNX_API_CONTROL_HEADERS
	ccnxControlAPI_About.h
	cpi_Acks.h
	cpi_Address.h
	cpi_AddressList.h
	cpi_CancelFlow.h
	cpi_Connection.h
	cpi_ConnectionEthernet.h
	cpi_ConnectionList.h
	cpi_ControlMessage.h
	cpi_ControlFacade.h
	cpi_Forwarding.h
	cpi_Interface.h
	cpi_InterfaceSet.h
	cpi_InterfaceGeneric.h
	cpi_InterfaceEthernet.h
	cpi_InterfaceType.h
	cpi_InterfaceIPTunnel.h
	cpi_InterfaceIPTunnelList.h
	cpi_InterfaceLocal.h
	cpi_Listener.h
	cpi_NameRouteType.h
	cpi_ManageLinks.h
	cpi_ManageCaches.h
	cpi_ManageWldr.h
	cpi_RouteEntry.h
	cpi_RouteEntryList.h
	cpi_NameRouteProtocolType.h
	cpi_ForwardingStrategy.h
	controlPlaneInterface.h
)

set(CCNX_API_CONTROL_SOURCE_FILES
	ccnxControlAPI_About.c
	cpi_Acks.c
	cpi_Address.c
	cpi_AddressList.c
	cpi_CancelFlow.c
	cpi_Connection.c
	cpi_ConnectionEthernet.c
	cpi_ConnectionList.c
	cpi_ControlMessage.c
	cpi_ControlFacade.c
	cpi_Forwarding.c
	cpi_Interface.c
	cpi_InterfaceSet.c
	cpi_InterfaceGeneric.c
	cpi_InterfaceEthernet.c
	cpi_InterfaceIPTunnel.c
	cpi_InterfaceIPTunnelList.c
	cpi_InterfaceType.c
	cpi_Listener.c
	cpi_NameRouteType.c
	cpi_ManageLinks.c
	cpi_ManageCaches.c
	cpi_ManageWldr.c
	cpi_NameRouteProtocolType.c
	cpi_RouteEntry.c
	cpi_RouteEntryList.c
	cpi_ForwardingStrategy.c
	controlPlaneInterface.c
)


add_library(ccnx_api_control         STATIC ${CCNX_API_CONTROL_SOURCE_FILES} ${CCNX_API_CONTROL_HEADERS})
add_library(ccnx_api_control.shared  SHARED ${CCNX_API_CONTROL_SOURCE_FILES})

source_group(Sources FILES ${CCNX_API_CONTROL_SOURCE_FILES})
source_group(Sources FILES ${CCNX_API_CONTROL_HEADERS})

set_target_properties(ccnx_api_control.shared PROPERTIES
  C_STANDARD 99
  SOVERSION 1
  VERSION 1.0
  OUTPUT_NAME ccnx_api_control )

set(libccnx_api_control_libraries
  ccnx_api_control
  ccnx_api_control.shared
  )

foreach(lib ${libccnx_api_control_libraries})
  install(TARGETS ${lib} COMPONENT library LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${CCNX_API_CONTROL_HEADERS} DESTINATION include/ccnx/api/control COMPONENT headers)

add_subdirectory(test)
