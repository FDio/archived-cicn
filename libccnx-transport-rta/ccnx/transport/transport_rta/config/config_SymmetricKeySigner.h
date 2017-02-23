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
 * @file config_SymmetricKeySigner.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the Symmetric Keystore.
 * The keystore is specific to a Connection, so there is no Protocol Stack configuration.
 *
 * @code
 * {
 *      // Configure a stack with {APIConnector,TLVCodec,MetisConnector}
 *      // The coded will use a symmetric keystore
 *
 *      stackConfig = ccnxStackConfig_Create();
 *      connConfig = ccnxConnectionConfig_Create();
 *
 *      apiConnector_ProtocolStackConfig(stackConfig);
 *      apiConnector_ConnectionConfig(connConfig);
 *      tlvCodec_ProtocolStackConfig(stackConfig);
 *      tlvCodec_ConnectionConfig(connConfig);
 *      symmetricKeySigner_ConnectionConfig(connConfig, "~/.ccnx/keystore.p12", "foobar password");
 *
 *      metisForwarder_ProtocolStackConfig(stackConfig);
 *      metisForwarder_ConnectionConfig(connConfig, metis_port);
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */

#ifndef Libccnx_config_SymmetricKeySigner_h
#define Libccnx_config_SymmetricKeySigner_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>
#include <stdbool.h>

struct symmetrickeysigner_params {
    const char *filename;
    const char *password;
};

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 * { "SIGNER" : "SymmetricFileStore",
 *   "SymmetricFileStore" : { "filename" : filename, password : password }
 * }
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
CCNxConnectionConfig *symmetricKeySigner_ConnectionConfig(CCNxConnectionConfig *config, const char *filename, const char *password);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *symmetricKeySigner_GetName(void);

/**
 * Look inside a JSON configuration and extract the Signer parameters
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [out] output The filename and password passed down the stack
 *
 * @return true Configuration item found and output set
 * @return false Configuration item not found, output not set
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool symmetricKeySigner_GetConnectionParams(PARCJSON *connectionJson, struct symmetrickeysigner_params *output);
#endif // Libccnx_config_SymmetricKeySigner_h
