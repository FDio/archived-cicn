# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_transport_MetaMessage 
	test_ccnx_ConnectionConfig 
	test_ccnx_StackConfig 
	test_ccnx_TransportConfig
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

