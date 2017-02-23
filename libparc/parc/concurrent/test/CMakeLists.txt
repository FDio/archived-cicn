set(TestsExpectedToPass
	test_parc_AtomicUint16
	test_parc_AtomicUint32
	test_parc_AtomicUint64
	test_parc_AtomicUint8
	test_parc_FutureTask
	test_parc_Lock
	test_parc_Notifier
	test_parc_RingBuffer_1x1
	test_parc_RingBuffer_NxM
	test_parc_ScheduledTask
	test_parc_ScheduledThreadPool
	test_parc_Synchronizer
	test_parc_Thread
	test_parc_ThreadPool
	test_parc_Timer
  )

# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()
