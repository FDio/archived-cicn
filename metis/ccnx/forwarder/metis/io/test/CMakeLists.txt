# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_AddressPair
	test_metis_EtherConnection
	test_metis_EtherListener
	test_metis_HopByHopFragmenter
	test_metis_IPMulticastListener
	test_metis_ListenerSet
	test_metis_LocalListener
	test_metis_StreamConnection
	test_metis_TcpListener
	test_metis_TcpTunnel
	test_metis_UdpConnection
	test_metis_UdpListener
	test_metis_UdpTunnel
)


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

