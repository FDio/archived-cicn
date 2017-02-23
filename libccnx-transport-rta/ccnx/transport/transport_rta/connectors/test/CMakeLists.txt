# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_connector_Api 
	test_rta_ApiConnection 
	test_connector_Forwarder_Local 
	test_connector_Forwarder_Metis
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

