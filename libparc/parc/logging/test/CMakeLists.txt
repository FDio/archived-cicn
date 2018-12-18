set(TestsExpectedToPass
  test_parc_Log
  test_parc_LogEntry
  test_parc_LogFormatSyslog
  test_parc_LogFormatText
  test_parc_LogLevel
  test_parc_LogReporter
  test_parc_LogReporterFile
  test_parc_LogReporterTextStdout
  )

# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()


