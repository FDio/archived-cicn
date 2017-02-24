# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_Connection
	test_metis_ConnectionTable
	test_metis_Dispatcher
        test_metis_Forwarder
	test_metis_Logger
	test_metis_Message
	test_metis_NumberSet
	test_metis_StreamBuffer
	test_metis_ConnectionList
	test_metis_ThreadedForwarder
)


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

