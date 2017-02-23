# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_LruList
	test_metis_ContentStoreEntry
	test_metis_ContentStoreInterface
	test_metis_TimeOrderedList
	test_metis_LRUContentStore
)


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

