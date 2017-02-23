# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
	test_codec_Signing 
	test_component_Codec_Tlv 
	test_component_Codec_Tlv_Hmac 
	test_component_Testing
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

