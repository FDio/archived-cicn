# Define a few configuration variables that we want accessible in the software

configure_file("config.h.in" "config.h" @ONLY)

set(LIBPARC_BASE_HEADER_FILES
	libparc_About.h
	)

set(LIBPARC_ALGOL_HEADER_FILES
    algol/parc_ArrayList.h
    algol/parc_AtomicInteger.h
    algol/parc_Base64.h
    algol/parc_BitVector.h
    algol/parc_Buffer.h
    algol/parc_BufferChunker.h
    algol/parc_BufferComposer.h
    algol/parc_BufferDictionary.h
    algol/parc_ByteArray.h
    algol/parc_Clock.h
    algol/parc_Chunker.h
    algol/parc_CMacro.h
    algol/parc_Collection.h
    algol/parc_Deque.h
    algol/parc_Dictionary.h
    algol/parc_DisplayIndented.h
    algol/parc_Environment.h
    algol/parc_Event.h
    algol/parc_EventScheduler.h
    algol/parc_EventSignal.h
    algol/parc_EventSocket.h
    algol/parc_EventTimer.h
    algol/parc_EventQueue.h
    algol/parc_EventBuffer.h
    algol/parc_Execution.h
    algol/parc_File.h
    algol/parc_FileChunker.h
    algol/parc_FileInputStream.h
    algol/parc_FileOutputStream.h
    algol/parc_Hash.h
    algol/parc_HashCode.h
    algol/parc_HashCodeTable.h
    algol/parc_HashMap.h
    algol/parc_InputStream.h
    algol/parc_Iterator.h
    algol/parc_JSON.h
    algol/parc_JSONArray.h
    algol/parc_JSONPair.h
    algol/parc_JSONValue.h
    algol/parc_JSONParser.h
    algol/parc_KeyValue.h
    algol/parc_KeyedElement.h
    algol/parc_List.h
    algol/parc_LinkedList.h
    algol/parc_Memory.h
    algol/parc_Network.h
    algol/parc_Object.h
    algol/parc_OutputStream.h
    algol/parc_PathName.h
    algol/parc_PriorityQueue.h
    algol/parc_Properties.h
    algol/parc_RandomAccessFile.h
    algol/parc_ReadOnlyBuffer.h
    algol/parc_StdlibMemory.h
    algol/parc_SafeMemory.h
    algol/parc_SortedList.h
    algol/parc_Stack.h
    algol/parc_String.h
    algol/parc_Time.h
    algol/parc_TreeMap.h
    algol/parc_TreeRedBlack.h
    algol/parc_URI.h
    algol/parc_URIAuthority.h
    algol/parc_URIPath.h
    algol/parc_URISegment.h
    algol/parc_Varint.h
    algol/parc_Vector.h
    algol/parc_Unsigned.h
	)

set(LIBPARC_PRIVATE_HEADER_FILES
	algol/internal_parc_Event.h
	)

set(LIBPARC_ALGOL_SOURCE_FILES
	libparc_About.c
	algol/parc_ArrayList.c
	algol/parc_AtomicInteger.c
	algol/parc_Base64.c
	algol/parc_BitVector.c
	algol/parc_Buffer.c
        algol/parc_BufferChunker.c
	algol/parc_BufferComposer.c
	algol/parc_BufferDictionary.c
	algol/parc_ByteArray.c
	algol/parc_Clock.c
        algol/parc_Chunker.c
	algol/parc_Deque.c
	algol/parc_Dictionary.c
	algol/parc_DisplayIndented.c
	algol/parc_Environment.c
	algol/parc_File.c
        algol/parc_FileChunker.c
	algol/parc_FileInputStream.c
	algol/parc_FileOutputStream.c
	algol/parc_Hash.c
	algol/parc_HashCode.c
	algol/parc_HashCodeTable.c
	algol/parc_InputStream.c
	algol/parc_Iterator.c
	algol/parc_JSON.c
	algol/parc_JSONArray.c
	algol/parc_JSONPair.c
	algol/parc_JSONValue.c
	algol/parc_JSONParser.c
	algol/parc_KeyValue.c
	algol/parc_KeyedElement.c
	algol/parc_List.c
	algol/parc_LinkedList.c
	algol/parc_Memory.c
	algol/internal_parc_Event.c
	algol/parc_Event.c
	algol/parc_EventScheduler.c
	algol/parc_EventSignal.c
	algol/parc_EventSocket.c
	algol/parc_EventTimer.c
	algol/parc_EventQueue.c
	algol/parc_EventBuffer.c
	algol/parc_Execution.c
	algol/parc_HashMap.c
	algol/parc_Network.c
	algol/parc_Object.c
	algol/parc_OutputStream.c
	algol/parc_PathName.c
    algol/parc_PriorityQueue.c
    algol/parc_Properties.c
    algol/parc_RandomAccessFile.c
	algol/parc_ReadOnlyBuffer.c
	algol/parc_SafeMemory.c
	algol/parc_SortedList.c
	algol/parc_StdlibMemory.c
    algol/parc_Stack.c
    algol/parc_String.c
	algol/parc_Time.c
	algol/parc_TreeMap.c
	algol/parc_TreeRedBlack.c
	algol/parc_URI.c
	algol/parc_URIAuthority.c
	algol/parc_URIPath.c
	algol/parc_URISegment.c
	algol/parc_Varint.c
	algol/parc_Vector.c
    algol/parc_Unsigned.c
	)

set(LIBPARC_TESTING_HEADER_FILES
	testing/parc_MemoryTesting.h
	testing/parc_ObjectTesting.h
	)

set(LIBPARC_TESTING_SOURCE_FILES
	testing/parc_MemoryTesting.c
	testing/parc_ObjectTesting.c
	)

set(LIBPARC_SECURITY_HEADER_FILES
	security/parc_CryptoHasher.h
	security/parc_CryptoHash.h
	security/parc_CryptoHashType.h
	security/parc_CryptoSuite.h
	security/parc_DiffieHellman.h
	security/parc_DiffieHellmanKeyShare.h
	security/parc_DiffieHellmanGroup.h
	security/parc_SigningAlgorithm.h
	security/parc_CryptoCache.h
	security/parc_InMemoryVerifier.h
	security/parc_Identity.h
	security/parc_IdentityFile.h
	security/parc_Key.h
	security/parc_KeyId.h
    security/parc_KeyStore.h
    security/parc_Pkcs12KeyStore.h
    security/parc_PublicKeySigner.h
	security/parc_SecureRandom.h
	security/parc_Security.h
	security/parc_Signature.h
	security/parc_Signer.h
    security/parc_SymmetricKeySigner.h
    security/parc_SymmetricKeyStore.h
	security/parc_Verifier.h
    security/parc_ContainerEncoding.h
    security/parc_Certificate.h
    security/parc_CertificateFactory.h
    security/parc_CertificateType.h
    security/parc_X509Certificate.h
	)

set(LIBPARC_SECURITY_SOURCE_FILES
	security/parc_CryptoHasher.c
	security/parc_CryptoHash.c
	security/parc_CryptoHashType.c
	security/parc_CryptoSuite.c
	security/parc_SigningAlgorithm.c
	security/parc_CryptoCache.c
	security/parc_DiffieHellman.c
	security/parc_DiffieHellmanKeyShare.c
	security/parc_InMemoryVerifier.c
	security/parc_Identity.c
	security/parc_IdentityFile.c
	security/parc_Key.c
	security/parc_KeyId.c
    security/parc_KeyStore.c
	security/parc_Pkcs12KeyStore.c
	security/parc_PublicKeySigner.c
    security/parc_SymmetricKeySigner.c
	security/parc_SymmetricKeyStore.c
	security/parc_Security.c
	security/parc_SecureRandom.c
	security/parc_Signature.c
	security/parc_Signer.c
	security/parc_Verifier.c
    security/parc_ContainerEncoding.c
    security/parc_Certificate.c
    security/parc_CertificateFactory.c
    security/parc_CertificateType.c
    security/parc_X509Certificate.c
	)


set(LIBPARC_CONCURRENT_HEADER_FILES
	concurrent/parc_AtomicUint16.h
	concurrent/parc_AtomicUint32.h
	concurrent/parc_AtomicUint64.h
	concurrent/parc_AtomicUint8.h
	concurrent/parc_FutureTask.h
	concurrent/parc_Lock.h
	concurrent/parc_Notifier.h
	concurrent/parc_RingBuffer.h
	concurrent/parc_RingBuffer_1x1.h
	concurrent/parc_RingBuffer_NxM.h
	concurrent/parc_ScheduledTask.h
	concurrent/parc_ScheduledThreadPool.h
	concurrent/parc_Synchronizer.h
	concurrent/parc_Thread.h
	concurrent/parc_ThreadPool.h
	concurrent/parc_Timeout.h
	concurrent/parc_Timer.h
	)

set(LIBPARC_CONCURRENT_SOURCE_FILES
	concurrent/parc_AtomicUint16.c
	concurrent/parc_AtomicUint32.c
	concurrent/parc_AtomicUint64.c
	concurrent/parc_AtomicUint8.c
	concurrent/parc_FutureTask.c
	concurrent/parc_Lock.c
	concurrent/parc_Notifier.c
	concurrent/parc_RingBuffer.c
	concurrent/parc_RingBuffer_1x1.c
	concurrent/parc_RingBuffer_NxM.c
	concurrent/parc_ScheduledTask.c
	concurrent/parc_ScheduledThreadPool.c
	concurrent/parc_Synchronizer.c
	concurrent/parc_Thread.c
	concurrent/parc_ThreadPool.c
	concurrent/parc_Timeout.c
	concurrent/parc_Timer.c
	)

set(LIBPARC_LOGGING_HEADER_FILES
	logging/parc_Log.h
	logging/parc_LogEntry.h
	logging/parc_LogLevel.h
	logging/parc_LogManager.h
	logging/parc_LogReporter.h
	logging/parc_LogReporterFile.h
	logging/parc_LogReporterTextStdout.h
	logging/parc_LogFormatText.h
	logging/parc_LogFormatSyslog.h
	)

set(LIBPARC_LOGGING_SOURCE_FILES
	logging/parc_Log.c
	logging/parc_LogEntry.c
	logging/parc_LogLevel.c
	logging/parc_LogManager.c
	logging/parc_LogReporter.c
	logging/parc_LogReporterFile.c
	logging/parc_LogReporterTextStdout.c
	logging/parc_LogFormatText.c
	logging/parc_LogFormatSyslog.c
	)

set(LIBPARC_DEVELOPER_HEADER_FILES
    developer/parc_TimingIntel.h
    developer/parc_Stopwatch.h
    developer/parc_Timing.h
	)

set(LIBPARC_DEVELOPER_SOURCE_FILES
    developer/parc_TimingIntel.c
    developer/parc_Stopwatch.c
	)

set(LIBPARC_STATISTICS_HEADER_FILES
    statistics/parc_BasicStats.h
    statistics/parc_EWMA.h
	)

set(LIBPARC_STATISTICS_SOURCE_FILES
    statistics/parc_BasicStats.c
    statistics/parc_EWMA.c
	)

set(LIBPARC_MEMORY_HEADER_FILES
    memory/parc_BufferPool.h
)

set(LIBPARC_MEMORY_SOURCE_FILES
    memory/parc_BufferPool.c
)

set(LIBPARC_SOURCE_FILES
    ${LIBPARC_ALGOL_SOURCE_FILES}
    ${LIBPARC_CONCURRENT_SOURCE_FILES}
	${LIBPARC_SECURITY_SOURCE_FILES}
	${LIBPARC_TESTING_SOURCE_FILES}
	${LIBPARC_LOGGING_SOURCE_FILES}
	${LIBPARC_DEVELOPER_SOURCE_FILES}
	${LIBPARC_BASE_HEADER_FILES}
	${LIBPARC_ALGOL_HEADER_FILES}
	${LIBPARC_PRIVATE_HEADER_FILES}
	${LIBPARC_TESTING_HEADER_FILES}
	${LIBPARC_SECURITY_HEADER_FILES}
	${LIBPARC_CONCURRENT_HEADER_FILES}
	${LIBPARC_CONCURRENT_SOURCE_FILES}
	${LIBPARC_LOGGING_HEADER_FILES}
	${LIBPARC_DEVELOPER_HEADER_FILES}
    ${LIBPARC_STATISTICS_HEADER_FILES} ${LIBPARC_STATISTICS_SOURCE_FILES}
    ${LIBPARC_MEMORY_HEADER_FILES} ${LIBPARC_MEMORY_SOURCE_FILES}
	)

set(LIBPARC_ALGOL_FILES ${LIBPARC_ALGOL_SOURCE_FILES} ${LIBPARC_ALGOL_HEADER_FILES})
list(SORT LIBPARC_ALGOL_FILES)
source_group(algol FILES ${LIBPARC_ALGOL_FILES})

#source_group(algol FILES ${LIBPARC_ALGOL_SOURCE_FILES} ${LIBPARC_ALGOL_HEADER_FILES})
source_group(concurrent FILES ${LIBPARC_CONCURRENT_SOURCE_FILES} ${LIBPARC_CONCURRENT_HEADER_FILES})
source_group(security FILES ${LIBPARC_SECURITY_SOURCE_FILES} ${LIBPARC_SECURITY_HEADER_FILES})
source_group(developer FILES ${LIBPARC_DEVELOPER_SOURCE_FILES} ${LIBPARC_DEVELOPER_HEADER_FILES})
source_group(logging FILES ${LIBPARC_LOGGING_SOURCE_FILES} ${LIBPARC_LOGGING_HEADER_FILES})
source_group(testing FILES ${LIBPARC_TESTING_SOURCE_FILES} ${LIBPARC_TESTING_HEADER_FILES})
source_group(statistics FILES ${LIBPARC_STATISTICS_SOURCE_FILES} ${LIBPARC_STATISTICS_HEADER_FILES})
source_group(memory FILES ${LIBPARC_MEMORY_SOURCE_FILES} ${LIBPARC_MEMORY_HEADER_FILES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR COMPILE_FOR_IOS)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
	message( "-- Set \"-undefined dynamic_lookup\" for shared libraries")
endif()
if(COMPILE_FOR_IOS OR ANDROID_API)
  add_library(parc         STATIC ${LIBPARC_SOURCE_FILES} ${LIBEVENT_LIBRARIES} ${OPENSSL_LIBRARIES})
  target_link_libraries(parc ${LIBEVENT_LIBRARIES})
  target_link_libraries(parc ${OPENSSL_LIBRARIES})
  set(libparcLibraries
    parc
    )
else()
  add_library(parc         STATIC ${LIBPARC_SOURCE_FILES})
  add_library(parc.shared  SHARED ${LIBPARC_SOURCE_FILES})
  target_link_libraries(parc.shared ${LIBEVENT_LIBRARIES})
  target_link_libraries(parc.shared ${OPENSSL_LIBRARIES})
  set_target_properties(parc.shared PROPERTIES
    C_STANDARD 99
    SOVERSION 1
    VERSION 1.0
    OUTPUT_NAME parc )
  set(libparcLibraries
    parc
    parc.shared
    )

endif()


foreach(lib ${libparcLibraries})
  install(TARGETS ${lib} COMPONENT library LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${LIBPARC_ALGOL_HEADER_FILES}      DESTINATION include/parc/algol COMPONENT headers)
install(FILES ${LIBPARC_SECURITY_HEADER_FILES}   DESTINATION include/parc/security COMPONENT headers)
install(FILES ${LIBPARC_LOGGING_HEADER_FILES}    DESTINATION include/parc/logging COMPONENT headers)
install(FILES ${LIBPARC_TESTING_HEADER_FILES}    DESTINATION include/parc/testing COMPONENT headers)
install(FILES ${LIBPARC_CONCURRENT_HEADER_FILES} DESTINATION include/parc/concurrent COMPONENT headers)
install(FILES ${LIBPARC_DEVELOPER_HEADER_FILES}  DESTINATION include/parc/developer COMPONENT headers)
install(FILES ${LIBPARC_BASE_HEADER_FILES}       DESTINATION include/parc COMPONENT headers)
install(FILES ${LIBPARC_STATISTICS_HEADER_FILES} DESTINATION include/parc/statistics COMPONENT headers)
install(FILES ${LIBPARC_MEMORY_HEADER_FILES}     DESTINATION include/parc/memory COMPONENT headers)

if(NOT COMPILE_FOR_IOS)
  add_subdirectory(security/command-line)
  add_subdirectory(algol/test)
  add_subdirectory(concurrent/test)
  add_subdirectory(developer/test)
  add_subdirectory(security/test)
  add_subdirectory(logging/test)
  add_subdirectory(testing/test)
  add_subdirectory(statistics/test)
  add_subdirectory(memory/test)
endif()
