# Define a few configuration variables that we want accessible in the software

configure_file(config.h.in config.h @ONLY)

set(METIS_BASE_HEADERS
	metis_About.h
	)

set(METIS_BASE_SOURCE
	metis_About.c
	)

source_group(core FILES ${METIS_BASE_HEADERS} ${METIS_BASE_SOURCE})

set(METIS_CONFIG_HEADERS
	config/metis_CommandOps.h
	config/metis_CommandParser.h
	config/metis_Configuration.h
	config/metis_CommandLineInterface.h
	config/metis_CommandReturn.h
	config/metis_SymbolicNameTable.h
	config/metis_ControlState.h
	config/metisControl_Root.h
	config/metisControl_AddConnection.h
	config/metisControl_Add.h
	config/metis_ConfigurationFile.h
	config/metis_ConfigurationListeners.h
	config/metisControl_AddRoute.h
	config/metisControl_AddListener.h
	config/metisControl_ListConnections.h
	config/metisControl_List.h
	config/metisControl_ListInterfaces.h
	config/metisControl_ListRoutes.h
	config/metisControl_Quit.h
	config/metisControl_Remove.h
	config/metisControl_RemoveConnection.h
	config/metisControl_RemoveRoute.h
	config/metisControl_Set.h
	config/metisControl_Unset.h
	config/metisControl_SetDebug.h
	config/metisControl_UnsetDebug.h
	config/metis_WebInterface.h
        config/metisControl_CacheServe.h
        config/metisControl_CacheStore.h
        config/metisControl_CacheClear.h
        config/metisControl_Cache.h
        config/metisControl_SetStrategy.h
        config/metisControl_SetWldr.h
	)

source_group(config FILES ${METIS_CONFIG_HEADERS})

set(METIS_CONFIG_SOURCE
	config/metis_CommandLineInterface.c
	config/metis_CommandOps.c
	config/metis_CommandParser.c
	config/metis_Configuration.c
	config/metis_ConfigurationFile.c
	config/metis_ConfigurationListeners.c
	config/metis_ControlState.c
	config/metis_SymbolicNameTable.c
	config/metisControl_Add.c
	config/metisControl_AddConnection.c
	config/metisControl_AddRoute.c
	config/metisControl_AddListener.c
	config/metisControl_List.c
	config/metisControl_ListConnections.c
	config/metisControl_ListInterfaces.c
	config/metisControl_ListRoutes.c
	config/metisControl_Quit.c
	config/metisControl_Remove.c
	config/metisControl_RemoveConnection.c
	config/metisControl_RemoveRoute.c
	config/metisControl_Root.c
	config/metisControl_Set.c
	config/metisControl_SetDebug.c
	config/metisControl_Unset.c
	config/metisControl_UnsetDebug.c
        config/metisControl_CacheServe.c
        config/metisControl_CacheStore.c
        config/metisControl_CacheClear.c
        config/metisControl_Cache.c
        config/metisControl_SetStrategy.c
        config/metisControl_SetWldr.c
	)

source_group(config FILES ${METIS_CONFIG_SOURCE})

set(METIS_CONTENT_STORE_HEADERS
	content_store/metis_ContentStoreEntry.h
	content_store/metis_ContentStoreInterface.h
	content_store/metis_LRUContentStore.h
	content_store/metis_TimeOrderedList.h
	content_store/metis_LruList.h
	)

source_group(content_store FILES ${METIS_CONTENT_STORE_HEADERS})

set(METIS_CONTENT_STORE_SOURCE
	content_store/metis_ContentStoreInterface.c
	content_store/metis_LRUContentStore.c
	content_store/metis_LruList.c
	content_store/metis_TimeOrderedList.c
	content_store/metis_ContentStoreEntry.c
	)

source_group(content_store FILES ${METIS_CONTENT_STORE_SOURCE})

set(METIS_CORE_HEADERS
	core/metis_ConnectionManager.h
	core/metis_Ticks.h
	core/metis_ConnectionList.h
	core/metis_ConnectionTable.h
	core/metis_Connection.h
	core/metis_Forwarder.h
	core/metis_Logger.h
	core/metis_Dispatcher.h
	core/metis_Message.h
	core/metis_MessagePacketType.h
	core/metis_NumberSet.h
	core/metis_StreamBuffer.h
	core/metis_ThreadedForwarder.h
	core/metis_System.h
        core/metis_Wldr.h
	)

source_group(core FILES ${METIS_CORE_HEADERS})

set(METIS_CORE_SOURCE
	core/metis_Connection.c
	core/metis_ConnectionList.c
	core/metis_ConnectionManager.c
	core/metis_ConnectionTable.c
	core/metis_Dispatcher.c
	core/metis_Forwarder.c
	core/metis_Logger.c
	core/metis_Message.c
	core/metis_NumberSet.c
	core/metis_StreamBuffer.c
	core/metis_ThreadedForwarder.c
        core/metis_Wldr.c
	)

source_group(core FILES ${METIS_CORE_SOURCE})

set(METIS_IO_HEADERS
	io/metis_HopByHopFragmenter.h
	io/metis_EtherConnection.h
	io/metis_TcpTunnel.h
	io/metis_AddressPair.h
	io/metis_IoOperations.h
	io/metis_Listener.h
	io/metis_ListenerSet.h
	io/metis_TcpListener.h
	io/metis_UdpTunnel.h
	io/metis_UdpConnection.h
	io/metis_UdpListener.h
	io/metis_EtherListener.h
	io/metis_GenericEther.h
	io/metis_LocalListener.h
	io/metis_StreamConnection.h
	io/metis_IPMulticastListener.h
	io/metis_Ethernet.h
	)

source_group(io FILES ${METIS_IO_HEADERS})

set(METIS_IO_SOURCE
	io/metis_AddressPair.c
	io/metis_EtherConnection.c
	io/metis_EtherListener.c
	io/metis_HopByHopFragmenter.c
	io/metis_IoOperations.c
	io/metis_IPMulticastListener.c
	io/metis_ListenerSet.c
	io/metis_LocalListener.c
	io/metis_StreamConnection.c
	io/metis_TcpListener.c
	io/metis_TcpTunnel.c
	io/metis_UdpConnection.c
	io/metis_UdpListener.c
	io/metis_UdpTunnel.c
	)

source_group(io FILES ${METIS_IO_SOURCE})

set(METIS_MESSENGER_HEADERS
	messenger/metis_MissiveDeque.h
	messenger/metis_Missive.h
	messenger/metis_MissiveType.h
	messenger/metis_Messenger.h
	messenger/metis_MessengerRecipient.h
	)

source_group(messenger FILES ${METIS_MESSENGER_HEADERS})

set(METIS_MESSENGER_SOURCE
	messenger/metis_Messenger.c
	messenger/metis_MessengerRecipient.c
	messenger/metis_Missive.c
	messenger/metis_MissiveDeque.c
	)

source_group(messenger FILES ${METIS_MESSENGER_SOURCE})

set(METIS_PROCESSOR_HEADERS
	processor/metis_FibEntry.h
	processor/metis_FibEntryList.h
	processor/metis_MessageProcessor.h
	processor/metis_Tap.h
	processor/metis_HashTableFunction.h
	processor/metis_PIT.h
	processor/metis_FIB.h
	processor/metis_PitEntry.h
	processor/metis_MatchingRulesTable.h
	processor/metis_PITVerdict.h
	processor/metis_StandardPIT.h
	)

source_group(processor FILES ${METIS_PROCESSOR_HEADERS})

set(METIS_PROCESSOR_SOURCE
	processor/metis_HashTableFunction.c
	processor/metis_FIB.c
	processor/metis_FibEntry.c
	processor/metis_FibEntryList.c
	processor/metis_MatchingRulesTable.c
	processor/metis_MessageProcessor.c
	processor/metis_PIT.c
	processor/metis_PitEntry.c
	processor/metis_StandardPIT.c
        )

source_group(processor FILES ${METIS_PROCESSOR_SOURCE})

set(METIS_STRATEGIES_HEADERS
	strategies/metis_Strategy.h
	strategies/metis_StrategyImpl.h
	strategies/strategy_LoadBalancer.h
	strategies/strategy_LoadBalancerWithPD.h
	strategies/strategy_NexthopState.h
	strategies/strategy_NexthopStateWithPD.h
	strategies/strategy_Rnd.h
	strategies/strategy_RndSegment.h
	)

source_group(strategies FILES ${METIS_STRATEGIES_HEADERS})

set(METIS_STRATEGIES_SOURCE
	strategies/metis_StrategyImpl.c
	strategies/strategy_LoadBalancer.c
	strategies/strategy_LoadBalancerWithPD.c
	strategies/strategy_NexthopState.c
	strategies/strategy_NexthopStateWithPD.c
	strategies/strategy_Rnd.c
	strategies/strategy_RndSegment.c
	)

source_group(strategies FILES ${METIS_STRATEGIES_SOURCE})

set(METIS_TLV_HEADERS
	tlv/metis_TlvOps.h
	tlv/metis_Tlv.h
	tlv/metis_TlvName.h
	tlv/metis_TlvExtent.h
	tlv/metis_TlvNameCodec.h
	tlv/metis_TlvSchemaV0.h
	tlv/metis_TlvSchemaV1.h
	tlv/metis_TlvSkeleton.h
	)

source_group(tlv FILES ${METIS_TLV_HEADERS})

set(METIS_TLV_SOURCE
	tlv/metis_Tlv.c
	tlv/metis_TlvExtent.c
	tlv/metis_TlvName.c
	tlv/metis_TlvSchemaV0.c
	tlv/metis_TlvSchemaV1.c
	tlv/metis_TlvSkeleton.c
	tlv/metis_TlvNameCodec.c
	)

source_group(tlv FILES ${METIS_TLV_SOURCE})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  set(METIS_PLATFORM_MODULES
      platforms/android/metis_GenericEther.c
      platforms/android/metis_System.c
      platforms/android/ifaddrs.c
     )
    source_group(platforms FILES ${METIS_PLATFORM_MODULES})
else()

if ( APPLE )
  set(METIS_PLATFORM_MODULES
	platforms/darwin/metis_GenericEther.c
	platforms/darwin/metis_System.c
	)
  source_group(platforms FILES ${METIS_PLATFORM_MODULES})
endif()


if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  set(METIS_PLATFORM_MODULES
    platforms/linux/metis_GenericEther.c
	platforms/linux/metis_System.c
	)
endif()
endif()

set(METIS_SOURCE_FILES
	metis_About.c
	${METIS_CONFIG_SOURCE}
	${METIS_CORE_SOURCE}
	${METIS_TLV_SOURCE}
	${METIS_CONTENT_STORE_SOURCE}
	${METIS_IO_SOURCE}
	${METIS_MESSENGER_SOURCE}
	${METIS_PLATFORM_MODULES}
	${METIS_PROCESSOR_SOURCE}
	${METIS_STRATEGIES_SOURCE}
	)

add_library(metis         STATIC ${METIS_SOURCE_FILES})

set(metis_libraries
  metis
  )

foreach(lib ${metis_libraries})
  install(TARGETS ${lib} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

if(NOT ANDROID_API)
    add_subdirectory(command-line)
else ()
	install(FILES ${METIS_BASE_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis)
 	install(FILES ${METIS_CONFIG_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/config)
	install(FILES ${METIS_CONTENT_STORE_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/content_store)
	install(FILES ${METIS_CORE_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/core)
	install(FILES ${METIS_IO_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/io)
	install(FILES ${METIS_MESSENGER_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/messenger)
	install(FILES ${METIS_PROCESSOR_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/processor)
	install(FILES ${METIS_STRATEGIES_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/strategies)
	install(FILES ${METIS_TLV_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccnx/forwarder/metis/tlv)
	install(FILES ${CMAKE_INSTALL_PREFIX}/../build/sb-forwarder/Metis/ccnx/forwarder/metis/config.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

endif()

add_subdirectory(test)
add_subdirectory(tlv/test)
add_subdirectory(strategies/test)
add_subdirectory(processor/test)
add_subdirectory(messenger/test)
add_subdirectory(io/test)
add_subdirectory(core/test)
add_subdirectory(content_store/test)
add_subdirectory(config/test)
