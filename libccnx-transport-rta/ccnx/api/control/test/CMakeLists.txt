# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_cpi_Acks 
	test_cpi_Address 
	test_cpi_AddressList 
	test_cpi_CancelFlow 
	test_cpi_Connection 
	test_cpi_ConnectionEthernet 
	test_cpi_ConnectionList 
	test_cpi_ControlMessage 
	test_cpi_ControlFacade 
	test_cpi_Interface 
	test_cpi_InterfaceSet 
	test_cpi_InterfaceTypes 
	test_cpi_InterfaceGeneric 
	test_cpi_InterfaceEthernet 
	test_cpi_InterfaceIPTunnel 
	test_cpi_InterfaceIPTunnelList 
	test_cpi_Forwarding 
	test_cpi_Listener 
	test_cpi_ManageLinks 
	test_cpi_NameRouteType 
	test_cpi_Registration 
	test_cpi_RouteEntry 
	test_cpi_RouteEntryList 
	test_controlPlaneInterface
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

