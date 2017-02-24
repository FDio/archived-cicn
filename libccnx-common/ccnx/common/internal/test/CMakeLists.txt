# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
  test_ccnx_ChunkingFacadeV1
  test_ccnx_ContentObjectFacadeV1
  test_ccnx_ContentObjectInterface
  test_ccnx_InterestFacadeV1
  test_ccnx_InterestInterface
  test_ccnx_InterestReturnFacadeV1
  test_ccnx_InterestReturnInterface
  test_ccnx_ManifestFacadeV1
  test_ccnx_ManifestInterface
  test_ccnx_TlvDictionary
  test_ccnx_ValidationFacadeV1
  test_ccnx_WireFormatFacadeV1
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

