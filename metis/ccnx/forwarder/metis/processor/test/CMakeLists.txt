# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_FIB
	test_metis_FibEntryList
	test_metis_HashTableFunction
	test_metis_FibEntry
        test_metis_MatchingRulesTable
	test_metis_MessageProcessor
	test_metis_PIT
	test_metis_PitEntry
	test_metis_StandardPIT
)


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

