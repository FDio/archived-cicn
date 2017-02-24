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
 * @file config_Codec_Tlv.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the TLV codec.
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

#ifndef Libccnx_config_Codec_Tlv_h
#define Libccnx_config_Codec_Tlv_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "TLV_CODEC" : { } }
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
CCNxStackConfig *tlvCodec_ProtocolStackConfig(CCNxStackConfig *stackConfig);

/**
 * Creates a connection configuration based on CCNxMessages wrapping an CCNxTlvDictionary
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
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
CCNxConnectionConfig *tlvCodec_ConnectionConfig(CCNxConnectionConfig *config);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *tlvCodec_GetName(void);

#endif
