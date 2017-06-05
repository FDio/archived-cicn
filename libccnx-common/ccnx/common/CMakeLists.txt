# Define a few configuration variables that we want accessible in the software

set(VALIDATION_HDRS
	validation/ccnxValidation_CRC32C.h
	validation/ccnxValidation_EcSecp256K1.h
	validation/ccnxValidation_HmacSha256.h
	validation/ccnxValidation_RsaSha256.h
	)

source_group(validation FILES ${VALIDATION_HDRS})

set(INTERNAL_HDRS
	internal/ccnx_ChunkingFacadeV1.h
	internal/ccnx_ContentObjectFacadeV1.h
	internal/ccnx_ContentObjectInterface.h
	internal/ccnx_InterestFacadeV1.h
	internal/ccnx_InterestDefault.h
	internal/ccnx_InterestInterface.h
	internal/ccnx_InterestReturnFacadeV1.h
	internal/ccnx_InterestReturnInterface.h
	internal/ccnx_InterestPayloadIdMethod.h
    internal/ccnx_ManifestFacadeV1.h
    internal/ccnx_ManifestInterface.h
	internal/ccnx_MessageInterface.h
	internal/ccnx_TlvDictionary.h
	internal/ccnx_ValidationFacadeV1.h
	internal/ccnx_WireFormatFacadeV1.h
	internal/ccnx_WireFormatMessageInterface.h
	)

source_group(internal FILES ${INTERNAL_HDRS})

set(CODEC_HDRS
	codec/ccnxCodec_EncodingBuffer.h
	codec/ccnxCodec_Error.h
	codec/ccnxCodec_ErrorCodes.h
	codec/ccnxCodec_NetworkBuffer.h
	codec/ccnxCodec_TlvEncoder.h
	codec/ccnxCodec_TlvDecoder.h
	codec/ccnxCodec_TlvUtilities.h
	codec/ccnxCodec_TlvPacket.h
	)

source_group(codec FILES ${CODEC_HDRS})

set(TESTDATA_HDRS
	codec/testdata/tlv_Schema.h
	codec/testdata/testdata_common.h
	)

set(SCHEMAV1_TESTDATA_HDRS
	codec/schema_v1/testdata/v1_content_nameA_crc32c.h
	codec/schema_v1/testdata/v1_content_nameA_keyid1_rsasha256.h
	codec/schema_v1/testdata/v1_content_no_payload.h
	codec/schema_v1/testdata/v1_ContentObjectSchema.h
	codec/schema_v1/testdata/v1_content_zero_payload.h
	codec/schema_v1/testdata/v1_cpi_add_route_crc32c.h
	codec/schema_v1/testdata/v1_cpi_add_route.h
	codec/schema_v1/testdata/v1_CPISchema.h
	codec/schema_v1/testdata/v1_interest_all_fields.h
	codec/schema_v1/testdata/v1_interest_bad_message_length.h
	codec/schema_v1/testdata/v1_interest_bad_validation_alg.h
	codec/schema_v1/testdata/v1_interest_nameA_badcrc32c.h
	codec/schema_v1/testdata/v1_interest_nameA_crc32c.h
	codec/schema_v1/testdata/v1_interest_nameA.h
	codec/schema_v1/testdata/v1_InterestSchema.h
	codec/schema_v1/testdata/v1_interest_validation_alg_overrun.h
	codec/schema_v1/testdata/v1_testrig_truthSet.h
	codec/schema_v1/testdata/v1_testrig_truthTable.h
	)

source_group(schema_v1_testData FILES ${SCHEMAV1_TESTDATA_HDRS})

set(SCHEMAV1_HDRS
	codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h
	codec/schema_v1/ccnxCodecSchemaV1_CryptoSuite.h
	codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderEncoder.h
	codec/schema_v1/ccnxCodecSchemaV1_FixedHeader.h
	codec/schema_v1/ccnxCodecSchemaV1_HashCodec.h
	codec/schema_v1/ccnxCodecSchemaV1_LinkCodec.h
	codec/schema_v1/ccnxCodecSchemaV1_ManifestDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_ManifestEncoder.h
	codec/schema_v1/ccnxCodecSchemaV1_MessageDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_MessageEncoder.h
	codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h
	codec/schema_v1/ccnxCodecSchemaV1_NameSegmentCodec.h
	codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersEncoder.h
	codec/schema_v1/ccnxCodecSchemaV1_PacketDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h
	codec/schema_v1/ccnxCodecSchemaV1_Types.h
	codec/schema_v1/ccnxCodecSchemaV1_ValidationDecoder.h
	codec/schema_v1/ccnxCodecSchemaV1_ValidationEncoder.h
	)

source_group(codec FILES ${SCHEMAV1_HDRS})

set(CORE_HDRS
	libccnxCommon_About.h
    ccnx_ContentObject.h
	ccnx_Interest.h
	ccnx_InterestReturn.h
	ccnx_InterestPayloadId.h
	ccnx_KeyLocator.h
	ccnx_KeystoreUtilities.h
	ccnx_Link.h
	ccnx_Manifest.h
    ccnx_ManifestHashGroup.h
	ccnx_Name.h
	ccnx_NameSegment.h
	ccnx_NameSegmentNumber.h
	ccnx_NameLabel.h
	ccnx_PayloadType.h
	ccnx_TimeStamp.h
	ccnx_WireFormatMessage.h
	)

source_group(core FILES ${CORE_HDRS})

set(ALL_HDRS
	${VALIDATION_HDRS}
	${INTERNAL_HDRS}
	${CODEC_HDRS}
	${TESTDATA_HDRS}
	${SCHEMAV1_TESTDATA_HDRS}
	${SCHEMAV1_HDRS}
	${CORE_HDRS})

set(VALIDATION_SRCS
	validation/ccnxValidation_CRC32C.c
	validation/ccnxValidation_EcSecp256K1.c
	validation/ccnxValidation_HmacSha256.c
	validation/ccnxValidation_RsaSha256.c
	)

source_group(validation FILES ${VALIDATION_SRCS})

set(INTERNAL_SRCS
	internal/ccnx_ChunkingFacadeV1.c
	internal/ccnx_ContentObjectFacadeV1.c
	internal/ccnx_ContentObjectInterface.c
	internal/ccnx_InterestFacadeV1.c
	internal/ccnx_InterestDefault.c
	internal/ccnx_InterestInterface.c
	internal/ccnx_InterestReturnFacadeV1.c
	internal/ccnx_InterestReturnInterface.c
    internal/ccnx_ManifestFacadeV1.c
    internal/ccnx_ManifestInterface.c
	internal/ccnx_TlvDictionary.c
	internal/ccnx_ValidationFacadeV1.c
	internal/ccnx_WireFormatFacadeV1.c
	internal/ccnx_WireFormatMessageInterface.c
	)

source_group(internal FILES ${INTERNAL_SRCS})

set(CODEC_V1_SRCS
	codec/schema_v1/ccnxCodecSchemaV1_CryptoSuite.c
	codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderEncoder.c
	codec/schema_v1/ccnxCodecSchemaV1_HashCodec.c
	codec/schema_v1/ccnxCodecSchemaV1_LinkCodec.c
	codec/schema_v1/ccnxCodecSchemaV1_ManifestDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_ManifestEncoder.c
	codec/schema_v1/ccnxCodecSchemaV1_MessageDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_MessageEncoder.c
	codec/schema_v1/ccnxCodecSchemaV1_NameCodec.c
	codec/schema_v1/ccnxCodecSchemaV1_NameSegmentCodec.c
	codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_OptionalHeadersEncoder.c
	codec/schema_v1/ccnxCodecSchemaV1_PacketDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.c
	codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.c
	codec/schema_v1/ccnxCodecSchemaV1_ValidationDecoder.c
	codec/schema_v1/ccnxCodecSchemaV1_ValidationEncoder.c
	)

source_group(codec FILES ${CODEC_V1_SRCS})

set(CODEC_SRCS
	codec/ccnxCodec_EncodingBuffer.c
	codec/ccnxCodec_Error.c
	codec/ccnxCodec_NetworkBuffer.c
	codec/ccnxCodec_TlvEncoder.c
	codec/ccnxCodec_TlvDecoder.c
	codec/ccnxCodec_TlvUtilities.c
	codec/ccnxCodec_TlvPacket.c
	${CODEC_V1_SRCS}
	)

source_group(codec FILES ${CODEC_SRCS})

set(CORE_SRCS
	libccnxCommon_About.c
    ccnx_ContentObject.c
	ccnx_Interest.c
	ccnx_InterestReturn.c
	ccnx_InterestPayloadId.c
	ccnx_KeyLocator.c
	ccnx_KeystoreUtilities.c
	ccnx_Link.c
	ccnx_Manifest.c
    ccnx_ManifestHashGroup.c
	ccnx_Name.c
	ccnx_NameSegment.c
	ccnx_NameSegmentNumber.c
	ccnx_NameLabel.c
	ccnx_TimeStamp.c
	ccnx_WireFormatMessage.c
	)

source_group(core FILES ${CORE_SRCS})

set(ALL_SRCS
	${INTERNAL_SRCS}
	${VALIDATION_SRCS}
	${CORE_SRCS}
	${CODEC_SRCS}
	${VALIDATION_HDRS}
	${INTERNAL_HDRS}
	${CODEC_HDRS}
	${TESTDATA_HDRS}
	${SCHEMAV1_TESTDATA_HDRS}
	${SCHEMAV1_HDRS}
	${COMMON_HDRS}
	)

set(PUBLIC_HDRS
	${CODEC_PRIVATE_HDRS}
)

configure_file("config.h.in" "config.h" @ONLY)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR COMPILE_FOR_IOS)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	message( "-- Set \"-undefined dynamic_lookup\" for shared libraries")
endif()

if(COMPILE_FOR_IOS OR ANDROID_API)
  add_library(ccnx_common         STATIC ${ALL_SRCS} ${ALL_HDRS})
  set(libccnx_commonLibraries
    ccnx_common
    )
else()
  add_library(ccnx_common         STATIC ${ALL_SRCS} ${ALL_HDRS})
  add_library(ccnx_common.shared  SHARED ${ALL_SRCS})

  target_link_libraries(ccnx_common.shared ${LIBPARC_LIBRARIES})
  set_target_properties(ccnx_common.shared PROPERTIES
    C_STANDARD 99
    SOVERSION 1
    VERSION 1.0
    OUTPUT_NAME ccnx_common )

  set(libccnx_commonLibraries
    ccnx_common
    ccnx_common.shared
    )
endif()

foreach(lib ${libccnx_commonLibraries})
  install(TARGETS ${lib} COMPONENT library LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${CORE_HDRS}               DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common COMPONENT headers)
install(FILES ${INTERNAL_HDRS}           DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/internal COMPONENT headers)
install(FILES ${VALIDATION_HDRS}         DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/validation COMPONENT headers)
install(FILES ${CODEC_HDRS}              DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/codec COMPONENT headers)
install(FILES ${TESTDATA_HDRS}           DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/codec/testdata COMPONENT headers)
install(FILES ${SCHEMAV1_HDRS}           DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/codec/schema_v1 COMPONENT headers)
install(FILES ${SCHEMAV1_TESTDATA_HDRS}  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/common/codec/schema_v1/testdata COMPONENT headers)

add_subdirectory(test)
add_subdirectory(validation/test)
add_subdirectory(codec/test)
add_subdirectory(internal/test)
add_subdirectory(codec/schema_v1/test)
