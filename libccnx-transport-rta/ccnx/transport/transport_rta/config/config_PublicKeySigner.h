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
 * @file config_PublicKeySigner.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for the PKCS12 Signer.
 *
 * The signer only as a Connection configuration.
 *
 * @code
 * {
 *      // Configure a stack with {APIConnector,TLVCodec,PKCS12Signer,MetisConnector}
 *
 *      stackConfig = ccnxStackConfig_Create();
 *      connConfig = ccnxConnectionConfig_Create();
 *
 *      apiConnector_ProtocolStackConfig(stackConfig);
 *      apiConnector_ConnectionConfig(connConfig);
 *      tlvCodec_ProtocolStackConfig(stackConfig);
 *      tlvCodec_ConnectionConfig(connConfig);
 *
 *      publicKeySigner_ConnectionConfig(connConfig, "~/.ccnx/keystore.p12", "foobar password");
 *
 *      metisForwarder_ProtocolStackConfig(stackConfig);
 *      metisForwarder_ConnectionConfig(connConfig, metisForwarder_GetDefaultPort());
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */
#ifndef Libccnx_config_PublicKeySigner_h
#define Libccnx_config_PublicKeySigner_h
#include <stdbool.h>

#include <parc/security/parc_Identity.h>

#include <ccnx/transport/common/ccnx_TransportConfig.h>

struct publickeysigner_params {
    const char *filename;
    const char *password;
};

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 * { "SIGNER" : "publicKeySigner",
 *   "publicKeySigner" : { "filename" : filename, "password" : password }
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
CCNxConnectionConfig *publicKeySigner_ConnectionConfig(CCNxConnectionConfig *config, const char *filename, const char *password);

/**
 * Generates the configuration settings included in the CCNxConnectionConfig for the identity of the configuration 'owner'
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 * { "SIGNER" : "publicKeySigner",
 *   "publicKeySigner" : { "filename" : filename, "password" : password }
 * }
 *
 * @param [in] connConfig The pointer to a valid CCNxConnectionConfig instance.
 * @param [in] identity A pointer to a valid PARCIdentity instance.
 *
 * @return non-null The modified `CCNxConnectionConfig`
 */
CCNxConnectionConfig *configPublicKeySigner_SetIdentity(CCNxConnectionConfig *connConfig, const PARCIdentity *identity);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *publicKeySigner_GetName(void);

/**
 * If successful, return true and fill in the parameters in the output argument
 *
 * Parses the JSON created by publicKeySigner_ConnectionConfig() and fills in the
 * output parameter.  The output parameter must be allocated by the caller.
 */
bool publicKeySigner_GetConnectionParams(PARCJSON *connectionJson, struct publickeysigner_params *output);
#endif // Libccnx_config_PublicKeySigner_h
