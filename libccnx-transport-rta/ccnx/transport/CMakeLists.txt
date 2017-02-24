set(BASE_HDRS
	librta_About.h
	)

set(COMMON_HDRS
	common/transport.h
	common/ccnx_TransportConfig.h
	common/transport_Message.h
	common/transport_MetaMessage.h
	common/ccnx_StackConfig.h
	common/ccnx_ConnectionConfig.h
	)

source_group(common FILES ${COMMON_HDRS})

set(RTA_CORE_HDRS
    transport_rta/rta_Transport.h
	transport_rta/core/components.h
	transport_rta/core/rta.h
	transport_rta/core/rta_ComponentQueue.h
	transport_rta/core/rta_ComponentStats.h
	transport_rta/core/rta_Connection.h
	transport_rta/core/rta_ConnectionTable.h
	transport_rta/core/rta_Framework.h
	transport_rta/core/rta_Framework_Commands.h
	transport_rta/core/rta_Framework_NonThreaded.h
	transport_rta/core/rta_Framework_Services.h
	transport_rta/core/rta_Framework_Threaded.h
	transport_rta/core/rta_Framework_private.h
	transport_rta/core/rta_Logger.h
	transport_rta/core/rta_ProtocolStack.h
	test_tools/bent_pipe.h
	test_tools/traffic_tools.h
	)

source_group(core FILES ${RTA_CORE_HDRS})

set(TEST_TOOLS_HDRS
	test_tools/bent_pipe.h
	test_tools/traffic_tools.h
  	)

set(RTA_COMMANDS_HDRS
	transport_rta/commands/rta_CommandCloseConnection.h
	transport_rta/commands/rta_CommandCreateProtocolStack.h
	transport_rta/commands/rta_CommandDestroyProtocolStack.h
	transport_rta/commands/rta_Command.h
	transport_rta/commands/rta_CommandOpenConnection.h
	transport_rta/commands/rta_CommandTransmitStatistics.h
	)

source_group(rta_commands FILES ${RTA_COMMANDS_HDRS})

set(RTA_CONFIG_HDRS
	transport_rta/config/config_ApiConnector.h
	transport_rta/config/config_Codec_Tlv.h
	transport_rta/config/config_CryptoCache.h
	transport_rta/config/config_FlowControl_Vegas.h
	transport_rta/config/config_Forwarder_Local.h
	transport_rta/config/config_Forwarder_Metis.h
	transport_rta/config/config_InMemoryVerifier.h
	transport_rta/config/config_ProtocolStack.h
	transport_rta/config/config_PublicKeySigner.h
	transport_rta/config/config_Signer.h
	transport_rta/config/config_SymmetricKeySigner.h
	transport_rta/config/config_TestingComponent.h
	)

source_group(rta_config FILES ${RTA_CONFIG_HDRS})

set(RTA_CONNECTORS_HDRS
	transport_rta/connectors/connector_Api.h
	transport_rta/connectors/rta_ApiConnection.h
	transport_rta/connectors/connector_Forwarder.h
	)

source_group(rta_connectors FILES ${RTA_CONNECTORS_HDRS})

set(RTA_COMPONENTS_HDRS
	transport_rta/components/Flowcontrol_Vegas/vegas_private.h
	transport_rta/components/codec_Signing.h
	transport_rta/components/component_Codec.h
	transport_rta/components/component_Flowcontrol.h
	transport_rta/components/component_Testing.h
	)

source_group(rta_components FILES ${RTA_COMPONENTS_HDRS})

set(COMMON_SRCS
	librta_About.c
	common/transport.c
	common/ccnx_TransportConfig.c
	common/transport_Message.c
	common/transport_MetaMessage.c
    common/ccnx_StackConfig.c
    common/ccnx_ConnectionConfig.c
	)

source_group(common FILES ${COMMON_SRCS})

set(RTA_CORE_SRCS
	transport_rta/core/rta_ComponentStats.c
	transport_rta/core/rta_Component.c
	transport_rta/core/rta_Connection.c
	transport_rta/core/rta_ConnectionTable.c
	transport_rta/core/rta_Framework.c
	transport_rta/core/rta_Framework_Commands.c
	transport_rta/core/rta_Framework_Services.c
	transport_rta/core/rta_Framework_Threaded.c
	transport_rta/core/rta_Framework_NonThreaded.c
	transport_rta/core/rta_Logger.c
	transport_rta/core/rta_ProtocolStack.c
	transport_rta/rta_Transport.c
	test_tools/bent_pipe.c
	test_tools/traffic_tools.c
	)

source_group(core FILES ${RTA_CORE_SRCS})

set(RTA_COMMANDS_SRCS
    transport_rta/commands/rta_Command.c
    transport_rta/commands/rta_CommandCloseConnection.c
    transport_rta/commands/rta_CommandCreateProtocolStack.c
    transport_rta/commands/rta_CommandDestroyProtocolStack.c
    transport_rta/commands/rta_CommandOpenConnection.c
    transport_rta/commands/rta_CommandTransmitStatistics.c
	)

source_group(rta_commands FILES ${RTA_COMMANDS_SRCS})

set(RTA_CONFIG_SRCS
	transport_rta/config/config_ApiConnector.c
	transport_rta/config/config_Codec_Tlv.c
	transport_rta/config/config_FlowControl_Vegas.c
	transport_rta/config/config_Forwarder_Local.c
	transport_rta/config/config_Forwarder_Metis.c
	transport_rta/config/config_TestingComponent.c
	transport_rta/config/config_InMemoryVerifier.c
	transport_rta/config/config_ProtocolStack.c
	transport_rta/config/config_PublicKeySigner.c
	transport_rta/config/config_Signer.c
	transport_rta/config/config_SymmetricKeySigner.c
	)

source_group(rta_config FILES ${RTA_CONFIG_SRCS})

set(RTA_CONNECTORS_SRCS
	transport_rta/connectors/connector_Api.c
	transport_rta/connectors/rta_ApiConnection.c
	transport_rta/connectors/connector_Forwarder_Local.c
	transport_rta/connectors/connector_Forwarder_Metis.c
	)

source_group(rta_connectors FILES ${RTA_CONNECTORS_SRCS})

set(RTA_COMPONENTS_SRCS
	transport_rta/components/codec_Signing.c
	transport_rta/components/component_Codec_Tlv.c
	transport_rta/components/Flowcontrol_Vegas/component_Vegas.c
	transport_rta/components/Flowcontrol_Vegas/vegas_Session.c
	transport_rta/components/component_Testing.c
	)

source_group(rta_components FILES ${RTA_COMPONENTS_SRCS})

set(TRANSPORT_RTA_SOURCE_FILES
  ${BASE_HDRS}
  ${COMMON_HDRS}
  ${RTA_CORE_HDRS}
  ${RTA_CONFIG_HDRS}
  ${RTA_COMPONENTS_HDRS}
  ${RTA_CONNECTORS_HDRS}
  ${RTA_COMMANDS_HDRS}
  ${TEST_TOOLS_HDRS}

  ${COMMON_SRCS}
  ${RTA_CORE_SRCS}
  ${RTA_CONFIG_SRCS}
  ${RTA_COMPONENTS_SRCS}
  ${RTA_CONNECTORS_SRCS}
  ${RTA_COMMANDS_SRCS}
  )

add_library(ccnx_transport_rta         STATIC ${TRANSPORT_RTA_SOURCE_FILES})
add_library(ccnx_transport_rta.shared  SHARED ${TRANSPORT_RTA_SOURCE_FILES})

set_target_properties(ccnx_transport_rta.shared PROPERTIES
  C_STANDARD 99
  SOVERSION 1
  VERSION 1.0
  OUTPUT_NAME ccnx_transport_rta )

set(libccnx_transport_rta_libraries
  ccnx_transport_rta
  ccnx_transport_rta.shared
  )

foreach(lib ${libccnx_transport_rta_libraries})
  install(TARGETS ${lib} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  set_property(TARGET ${lib} PROPERTY C_STANDARD 99)
endforeach()

install(FILES ${BASE_HDRS} DESTINATION include/ccnx/transport )
install(FILES ${COMMON_HDRS} DESTINATION include/ccnx/transport/common )
install(FILES ${TEST_TOOLS_HDRS} DESTINATION include/ccnx/transport/test_tools )
install(FILES ${RTA_CORE_HDRS} DESTINATION include/ccnx/transport/transport_rta )
install(FILES ${RTA_CONFIG_HDRS} DESTINATION include/ccnx/transport/transport_rta/config )
install(FILES ${RTA_COMMANDS_HDRS} DESTINATION include/ccnx/transport/transport_rta/commands )

add_subdirectory(common/test)
add_subdirectory(transport_rta/test)
add_subdirectory(transport_rta/commands/test)
add_subdirectory(transport_rta/components/test)
add_subdirectory(transport_rta/config/test)
add_subdirectory(transport_rta/connectors/test)
add_subdirectory(transport_rta/core/test)
