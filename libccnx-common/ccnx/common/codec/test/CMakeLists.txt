# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

configure_file(test_rsa.p12 test_rsa.p12 COPYONLY)
configure_file(test_random_bytes test_random_bytes COPYONLY) 
configure_file(test_random_bytes.sig test_random_bytes.sig COPYONLY)
configure_file(test_rsa_key.pem test_rsa_key.pem COPYONLY)

set(TestsExpectedToPass
  test_ccnxCodec_EncodingBuffer
  test_ccnxCodec_Error
  test_ccnxCodec_NetworkBuffer
  test_ccnxCodec_TlvDecoder
  test_ccnxCodec_TlvEncoder
  test_ccnxCodec_TlvPacket
  test_ccnxCodec_TlvUtilities
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

