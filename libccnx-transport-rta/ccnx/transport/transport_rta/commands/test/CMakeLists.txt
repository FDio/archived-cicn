# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_rta_Command 
	test_rta_CommandCreateProtocolStack 
	test_rta_CommandOpenConnection 
	test_rta_CommandCloseConnection 
	test_rta_CommandDestroyProtocolStack 
	test_rta_CommandTransmitStatistics
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

