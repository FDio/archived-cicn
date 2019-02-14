# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_ccnx_PortalFactory
   	test_ccnx_PortalAPI
)

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

