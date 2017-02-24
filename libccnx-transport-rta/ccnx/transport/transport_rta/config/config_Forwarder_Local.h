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
 * @file config_Forwarder_Local.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the local testing forwarder.
 *
 * The local forwarder requires one parameter for the path to the unix socket.
 *
 * @code
 * {
 *      // Configure a stack with {APIConnector,TLVCodec,LocalForwarder}
 *
 *      stackConfig = ccnxStackConfig_Create();
 *      connConfig = ccnxConnectionConfig_Create();
 *
 *      apiConnector_ProtocolStackConfig(stackConfig);
 *      apiConnector_ConnectionConfig(connConfig);
 *      tlvCodec_ProtocolStackConfig(stackConfig);
 *      tlvCodec_ConnectionConfig(connConfig);
 *      localForwarder_ProtocolStackConfig(stackConfig);
 *      localForwarder_ConnectionConfig(connConfig, "/var/run/bentpipe.sock");
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */

#ifndef Libccnx_config_Forwarder_Local_h
#define Libccnx_config_Forwarder_Local_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "FWD_LOCAL" : {  } }
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
CCNxStackConfig *localForwarder_ProtocolStackConfig(CCNxStackConfig *stackConfig);

/**
 * Generates:
 *
 */

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 *  { "FWD_LOCAL" : { "path" : pipePath } }
 *
 * @param [in] config A pointer to a valid CCNxConnectionConfig instance.
 * @param [in] pipePath The filesystem path to the unix domain socket
 *
 * @return non-null The modified `CCNxConnectionConfig`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxConnectionConfig *localForwarder_ConnectionConfig(CCNxConnectionConfig *config, const char *pipePath);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *localForwarder_GetName(void);

const char *localForwarder_GetPath(PARCJSON *connectionJson);
#endif // Libccnx_config_Forwarder_Local_h
