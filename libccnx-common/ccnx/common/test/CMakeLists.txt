# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

configure_file(data.json data.json COPYONLY)

set(TestsExpectedToPass
  test_ccnx_ContentObject
  test_ccnx_Interest
  test_ccnx_InterestPayloadId
  test_ccnx_InterestReturn
  test_ccnx_KeyLocator
  test_ccnx_KeystoreUtilities
  test_ccnx_Link
  test_ccnx_Manifest
  test_ccnx_ManifestHashGroup
  test_ccnx_Name
  test_ccnx_NameLabel
  test_ccnx_NameSegment
  test_ccnx_NameSegmentNumber
  test_ccnx_TimeStamp
  test_ccnx_WireFormatMessage
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

