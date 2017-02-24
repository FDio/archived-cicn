# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

set(TestsExpectedToPass
  test_strategy_All
)

  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()

