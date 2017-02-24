# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
  test_ccnxValidation_CRC32C
  test_ccnxValidation_EcSecp256K1
  test_ccnxValidation_HmacSha256
  test_ccnxValidation_RsaSha256
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

