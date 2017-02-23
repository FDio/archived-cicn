# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_sys_Errors
	test_sys_EtherEndToEnd
	test_sys_TcpEndToEnd
	test_sys_UdpEndToEnd
	test_sys_TcpTunnel
)

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

