# Define a few configuration variables that we want accessible in the software
configure_file(
  config.h.in
  config.h
)

set(LONGBOW_REPORT_HEADER_FILES
  Reporting/longBowReport_Testing.h
  Reporting/longBowReport_Runtime.h
  )

set(LONGBOW_REPORT_BASE_SOURCE_FILES
    Reporting/longBowReport_Runtime.c
    Reporting/longBowReport_Testing.c
	${LONGBOW_REPORT_HEADER_FILES})

source_group(report-base FILES ${LONGBOW_REPORT_BASE_SOURCE_FILES})

set(LONGBOW_REPORT_ANSI_SOURCE_FILES
    Reporting/ANSITerm/longBowReport_Runtime.c
    Reporting/ANSITerm/longBowReport_Runtime.h
    Reporting/ANSITerm/longBowReport_Testing.c
    Reporting/ANSITerm/longBowReportANSITerminal_About.c
    Reporting/ANSITerm/longBowReportANSITerminal_About.h)

source_group(report-ansiterm FILES ${LONGBOW_REPORT_ANSI_SOURCE_FILES})


if (ANDROID_API)
  set(LONGBOW_REPORT_TEXT_RUNTIME_SOURCE_FILE Reporting/Android/longBowReport_Runtime.c)
else()
  set(LONGBOW_REPORT_TEXT_RUNTIME_SOURCE_FILE Reporting/TextPlain/longBowReport_Runtime.c)
endif()

  set(LONGBOW_REPORT_TEXT_SOURCE_FILES
      ${LONGBOW_REPORT_TEXT_RUNTIME_SOURCE_FILE}
      Reporting/TextPlain/longBowReport_Testing.c
      Reporting/TextPlain/longBowReportTextPlain_About.c
      Reporting/TextPlain/longBowReportTextPlain_About.h)

source_group(report-textplain FILES ${LONGBOW_REPORT_TEXT_SOURCE_FILES})

set(LONGBOW_CONFIG_SOURCE_FILES
	command-line/liblongbow-config.c)

source_group(longbow-config FILES ${LONGBOW_CONFIG_SOURCE_FILES})

set(LIBLONGBOW_HEADER_FILES
        stubs/execinfo.h
        assertions.h
        debugging.h
        longBow_About.h
        longBow_Backtrace.h
        longBow_ClipBoard.h
        longBow_Compiler.h
        longBow_Config.h
        longBow_Debug.h
        longBow_Event.h
        longBow_EventType.h
        longBow_Location.h
        longBow_Main.h
        longBow_MeasureTime.h
        longBow_Properties.h
        longBow_Runtime.h
        longBow_RuntimeResult.h
        longBow_Status.h
        longBow_SubProcess.h
        longBow_TestCase.h
        longBow_TestCaseClipBoard.h
		longBow_TestCaseMetaData.h
        longBow_TestFixture.h
        longBow_TestFixtureConfig.h
        longBow_TestRunner.h
        longBow_UnitTest.h
        longBow_UnitTesting.h
        runtime.h
        testing.h
        tests.h
        traps.h
        unit-test.h
		)

set(LIBLONGBOW_SOURCE_FILES
        private/longBow_ArrayList.h
        private/longBow_Memory.h
        private/longBow_OpenFile.h
        private/longBow_String.h
        private/longBow_ArrayList.c
        private/longBow_Memory.c
        private/longBow_OpenFile.c
        private/longBow_String.c
        longBow_About.c
        longBow_Backtrace.c
	longBow_ClipBoard.c
        longBow_Config.c
        longBow_Debug.c
        longBow_Event.c
        longBow_EventType.c
        longBow_Location.c
        longBow_Main.c
        longBow_MeasureTime.c
        longBow_Properties.c
        longBow_Runtime.c
        longBow_RuntimeResult.c
        longBow_Status.c
        longBow_SubProcess.c
        longBow_TestCase.c
        longBow_TestCaseClipBoard.c
	longBow_TestCaseMetaData.c
        longBow_TestFixture.c
        longBow_TestFixtureConfig.c
        longBow_TestRunner.c
        longBow_UnitTesting.c
	${LIBLONGBOW_HEADER_FILES})

source_group(longbow FILES ${LIBLONGBOW_SOURCE_FILES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	message( "-- Set \"-undefined dynamic_lookup\" for shared libraries")
endif()

add_library(longbow-ansiterm STATIC ${LONGBOW_REPORT_ANSI_SOURCE_FILES} ${LONGBOW_REPORT_BASE_SOURCE_FILES})
add_library(longbow-ansiterm.shared SHARED ${LONGBOW_REPORT_ANSI_SOURCE_FILES} ${LONGBOW_REPORT_BASE_SOURCE_FILES})

set_target_properties(longbow-ansiterm.shared PROPERTIES 
  SOVERSION 1 
  VERSION 1.0
  OUTPUT_NAME longbow-ansiterm)

add_library(longbow-textplain STATIC ${LONGBOW_REPORT_TEXT_SOURCE_FILES} ${LONGBOW_REPORT_BASE_SOURCE_FILES})
add_library(longbow-textplain.shared SHARED ${LONGBOW_REPORT_TEXT_SOURCE_FILES} ${LONGBOW_REPORT_BASE_SOURCE_FILES})

set_target_properties(longbow-textplain.shared PROPERTIES 
  SOVERSION 1 
  VERSION 1.0
  OUTPUT_NAME longbow-textplain)

add_library(longbow STATIC ${LIBLONGBOW_SOURCE_FILES})
add_library(longbow.shared SHARED ${LIBLONGBOW_SOURCE_FILES})

set_target_properties(longbow.shared PROPERTIES 
  SOVERSION 1 
  VERSION 1.0
  OUTPUT_NAME longbow)

add_executable(longbow-config ${LONGBOW_CONFIG_SOURCE_FILES})


set(longbowLibraries
  longbow
  longbow.shared
  longbow-ansiterm
  longbow-ansiterm.shared
  longbow-textplain
  longbow-textplain.shared
  )

foreach(lib ${longbowLibraries})
  install(TARGETS ${lib} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
endforeach()

install(FILES ${LIBLONGBOW_HEADER_FILES}     DESTINATION include/LongBow )
install(FILES ${LONGBOW_REPORT_HEADER_FILES} DESTINATION include/LongBow/Reporting )

if(ANDROID_API)
  message("############ Detected cross compile for ${CMAKE_SYSTEM_NAME}")
  message("############ This build will not include doxygen, tools, or tests")
else()
  add_subdirectory(test)
endif()
