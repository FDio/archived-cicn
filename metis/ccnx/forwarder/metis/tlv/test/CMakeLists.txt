# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_metis_Tlv 
	test_metis_TlvExtent 
	test_metis_TlvName 
	test_metis_TlvNameCodec 
	test_metis_TlvSchemaV0 
	test_metis_TlvSchemaV1 
	test_metis_TlvSkeleton
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

