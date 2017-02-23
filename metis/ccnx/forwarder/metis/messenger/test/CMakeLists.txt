# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_Missive 
	test_metis_MissiveDeque 
	test_metis_Messenger 
	test_metis_MessengerRecipient
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

