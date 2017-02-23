# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")

configure_file(data.json data.json COPYONLY)

set(TestsExpectedToPass
  test_parc_ArrayList
  test_parc_AtomicInteger
  test_parc_Base64
  test_parc_BitVector
  test_parc_Buffer
  test_parc_BufferChunker
  test_parc_BufferComposer
  test_parc_ByteArray
  test_parc_Clock
  test_parc_Chunker
  test_parc_Deque
  test_parc_Dictionary
  test_parc_Display
  test_parc_Environment
  test_parc_Event
  test_parc_EventBuffer
  test_parc_EventQueue
  test_parc_EventScheduler
  test_parc_EventSignal
  test_parc_EventSocket
  test_parc_EventTimer
  test_parc_File
  test_parc_FileChunker
  test_parc_FileInputStream
  test_parc_FileOutputStream
  test_parc_Hash
  test_parc_HashCode
  test_parc_HashCodeTable
  test_parc_HashMap
  test_parc_InputStream
  test_parc_Iterator
  test_parc_JSON
  test_parc_JSONArray
  test_parc_JSONPair
  test_parc_JSONParser
  test_parc_JSONValue
  test_parc_KeyValue
  test_parc_KeyedElement
  test_parc_LinkedList
  test_parc_List
  test_parc_Memory
  test_parc_Network
  test_parc_Object
  test_parc_PathName
  test_parc_PriorityQueue
  test_parc_Properties
  test_parc_RandomAccessFile
  test_parc_ReadOnlyBuffer
  test_parc_SafeMemory
  test_parc_SortedList
  test_parc_Stack
  test_parc_StdlibMemory
  test_parc_String
  test_parc_Time
  test_parc_TreeMap
  test_parc_TreeRedBlack
  test_parc_URI
  test_parc_URIAuthority
  test_parc_URIPath
  test_parc_URISegment
  test_parc_Varint
  test_parc_Vector
  )
  
foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()


