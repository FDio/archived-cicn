# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_ccnx_Portal
	test_ccnx_PortalFactory
	test_ccnx_PortalStack
   	test_ccnx_PortalAPI
	test_ccnx_PortalRTA
	test_ccnx_PortalAnchor
)

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

