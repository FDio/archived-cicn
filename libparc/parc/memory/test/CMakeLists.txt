set(TestsExpectedToPass
    test_parc_BufferPool
  )

# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()


