# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_ControlState
	#test_metis_CommandLineInterface
	test_metis_CommandOps
	test_metis_CommandParser
	test_metis_Configuration
	test_metis_ConfigurationFile
	test_metis_ConfigurationListeners
	test_metis_SymbolicNameTable
	test_metisControl_Add
	test_metisControl_AddConnection
	test_metisControl_AddListener
        test_metisControl_AddRoute
	test_metisControl_List
	test_metisControl_ListConnections
	test_metisControl_ListInterfaces
	test_metisControl_ListRoutes
	test_metisControl_Quit
	test_metisControl_Remove
	test_metisControl_RemoveConnection
	test_metisControl_RemoveRoute
	test_metisControl_Root
	test_metisControl_Set
	test_metisControl_SetDebug
	test_metisControl_Unset
	test_metisControl_UnsetDebug
)

 
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

