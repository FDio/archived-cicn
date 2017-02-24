# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
  test_ccnxCodecSchemaV1_CryptoSuite
  test_ccnxCodecSchemaV1_FixedHeaderDecoder
  test_ccnxCodecSchemaV1_FixedHeaderEncoder
  test_ccnxCodecSchemaV1_LinkCodec
  test_ccnxCodecSchemaV1_MessageDecoder
  test_ccnxCodecSchemaV1_MessageEncoder
  test_ccnxCodecSchemaV1_NameCodec
  test_ccnxCodecSchemaV1_NameSegmentCodec
  test_ccnxCodecSchemaV1_OptionalHeadersDecoder
  test_ccnxCodecSchemaV1_OptionalHeadersEncoder
  test_ccnxCodecSchemaV1_PacketDecoder
  test_ccnxCodecSchemaV1_PacketEncoder
  test_ccnxCodecSchemaV1_TlvDictionary
  test_ccnxCodecSchemaV1_ValidationDecoder
  test_ccnxCodecSchemaV1_ValidationEncoder
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

