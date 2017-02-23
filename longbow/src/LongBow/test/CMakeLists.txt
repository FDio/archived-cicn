# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")


macro(AddTest testFile)
  add_executable(${ARGV0} ${ARGV0}.c)
  target_link_libraries(${ARGV0} longbow longbow-ansiterm)
  add_test(${ARGV0} ${ARGV0})
  set_target_properties(${ARGV0} PROPERTIES FOLDER Test)
endmacro(AddTest)

set(TestsExpectedToFail
    test_longBow_Expected;
)

#    test_assertions;

set(TestsExpectedToPass
    test_MemoryLeaks;
    test_longBow_ArrayList;
    test_longBow_Config;
    test_longBow_Fixture;
    test_longBow_Memory;
    test_longBow_Location;
    test_fixtureConfiguration;
    test_longBow_Backtrace;
    test_longBow_CommandLineOptions;
    test_longBow_Debug;
    test_longBow_Main;
    test_longBow_MeasureTime;
    test_longBow_OpenFile;
    test_longBow_Properties;
    test_longBow_Runner;
    test_longBow_Runtime;
    test_longBow_Status;
    test_longBow_String;
    test_longBow_TestCase;
    test_longBow_TestCaseClipBoard;
    test_longBow_UnitTesting
   )

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

foreach(test ${TestsExpectedToFail})
   AddTest(${test})
   set_tests_properties(${test} PROPERTIES WILL_FAIL TRUE)
endforeach()


