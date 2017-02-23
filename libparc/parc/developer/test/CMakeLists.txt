set(TestsExpectedToPass
  test_parc_Stopwatch
  test_parc_Timing
  )

# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()


