/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file config_ProtocolStack.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the ProtocolStack.
 *
 * The ProtocolStack configuration is a list of key names for the components
 * in the stack.  It is an in-order list of the components to configure in the
 * stack.
 *
 * @code
 * {
 *      // Configure a stack with {APIConnector,TLVCodec,MetisConnector}
 *
 *      stackConfig = ccnxStackConfig_Create();
 *      connConfig = ccnxConnectionConfig_Create();
 *
 *      apiConnector_ProtocolStackConfig(stackConfig);
 *      apiConnector_ConnectionConfig(connConfig);
 *      tlvCodec_ProtocolStackConfig(stackConfig);
 *      tlvCodec_ConnectionConfig(connConfig);
 *      inMemoryVerifier_ConnectionConfig(connConfig);
 *      metisForwarder_ProtocolStackConfig(stackConfig);
 *      metisForwarder_ConnectionConfig(connConfig, metisForwarder_GetDefaultPort());
 *
 *      protocolStack_ComponentsConfigArgs(stackConfig, apiConnector_Name(), tlvCodec_Name(), metisForwarder_Name(), NULL);
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */

#ifndef Libccnx_config_ProtocolStack_h
#define Libccnx_config_ProtocolStack_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>
#include <parc/algol/parc_ArrayList.h>

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "COMPONENTS" : [ name1, name2, ... ] }
 *
 * The ProtocolStack function adds a configuration element that enumerates each component
 * that will be in the protocol stack, in order.  These names must match the names
 * used by each component in its own particular configuration.
 *
 * @param [in] stackConfig The protocl stack configuration to update
 *
 * @return non-null The updated protocol stack configuration
 *
 * Example:
 * @code
 * {
 *      protocolStack_ComponentsConfigArgs(stackConfig, apiConnector_Name(), tlvCodec_Name(), metisForwarder_Name(), NULL);
 * }
 * @endcode
 */
CCNxStackConfig *protocolStack_ComponentsConfigArgs(CCNxStackConfig *stackConfig, ...);

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "COMPONENTS" : [ name1, name2, ... ] }
 *
 * The ProtocolStack function adds a configuration element that enumerates each component
 * that will be in the protocol stack, in order.  These names must match the names
 * used by each component in its own particular configuration.
 *
 * @param [in] stackConfig The protocl stack configuration to update
 *
 * @return non-null The updated protocol stack configuration
 *
 * Example:
 * @code
 * @endcode
 */
CCNxStackConfig *protocolStack_ComponentsConfigArrayList(CCNxStackConfig *stackConfig, const PARCArrayList *listOfComponentNames);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *protocolStack_GetName(void);

/**
 * Parse the protocol stack json to extract an array list of the component names
 */
PARCArrayList *protocolStack_GetComponentNameArray(PARCJSON *stackJson);
#endif // Libccnx_config_ProtocolStack_h
