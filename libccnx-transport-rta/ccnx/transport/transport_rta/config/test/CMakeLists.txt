# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_config_ApiConnector
	test_config_Codec_Tlv
	test_config_FlowControl_Vegas
	test_config_Forwarder_Local
	test_config_Forwarder_Metis
	test_config_InMemoryVerifier
	test_config_ProtocolStack
	test_config_PublicKeySigner
	test_config_Signer
	test_config_SymmetricKeySigner
	test_config_TestingComponent
)


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()
