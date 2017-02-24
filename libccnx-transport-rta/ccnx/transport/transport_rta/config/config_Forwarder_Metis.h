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
 * @file config_Forwarder_Metis.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the Metis connector.
 *
 * The Metis connector requires one parameter to specify the port.
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
 *      metisForwarder_ProtocolStackConfig(stackConfig);
 *      metisForwarder_ConnectionConfig(connConfig, metisForwarder_GetDefaultPort());
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */

#ifndef Libccnx_config_Forwarder_Metis_h
#define Libccnx_config_Forwarder_Metis_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>

#define METIS_PORT_ENV "METIS_PORT"
#define FORWARDER_CONNECTION_ENV "CCNX_FORWARDER"

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "FWD_METIS" : { } }
 *
 * @param [in] stackConfig The protocl stack configuration to update
 *
 * @return non-null The updated protocol stack configuration
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxStackConfig *metisForwarder_ProtocolStackConfig(CCNxStackConfig *stackConfig);

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 *  { "FWD_METIS" : { "port" : port } }
 *
 * @param [in] config A pointer to a valid CCNxConnectionConfig instance.
 *
 * @return non-null The modified `CCNxConnectionConfig`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxConnectionConfig *metisForwarder_ConnectionConfig(CCNxConnectionConfig *config, uint16_t port);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *metisForwarder_GetName(void);

/**
 * Returns the IANA assigned port for the CCN forwarder
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return 9695 The IANA assigned port for CCN
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint16_t metisForwarder_GetDefaultPort(void);

/**
 * Return the metis port ot use (or the default 9695) based on the setting
 * in the per-connection configuration
 */
uint16_t metisForwarder_GetPortFromConfig(PARCJSON *json);

#endif // Libccnx_config_Forwarder_Metis_h
