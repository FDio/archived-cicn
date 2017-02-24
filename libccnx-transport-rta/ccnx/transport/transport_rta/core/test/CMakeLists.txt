# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_rta_ConnectionTable 
	test_rta_Framework 
	test_rta_Framework_Commands 
	test_rta_Component 
	test_rta_Connection 
	test_rta_Framework_NonThreaded 
	test_rta_Framework_Services 
	test_rta_Framework_Threaded 
	test_rta_Logger 
	test_rta_ProtocolStack 
	test_rta_ComponentStats
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

