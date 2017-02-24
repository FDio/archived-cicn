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
 * @file config_TestingComponent.h
 * @brief Generates stack and connection configuration information
 *
 * Each component in the protocol stack must have a configuration element.
 * This module generates the configuration elements for a testing component
 * to be used in unit tests.  The upper and lower testing components surround
 * a component under test to simulate feeding in or sending out messages.
 *
 * @code
 * {
 *      // Configure a stack with {TestingUpper,TLVCodec,TestingLower}
 *
 *      stackConfig = ccnxStackConfig_Create();
 *      connConfig = ccnxConnectionConfig_Create();
 *
 *      testingUpper_ProtocolStackConfig(stackConfig);
 *      testingUpper_ConnectionConfig(connConfig);
 *      tlvCodec_ProtocolStackConfig(stackConfig);
 *      tlvCodec_ConnectionConfig(connConfig);
 *      testingLower_ProtocolStackConfig(stackConfig);
 *      testingLower_ConnectionConfig(connConfig, metis_port);
 *
 *      CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connConfig);
 * }
 *
 */

#ifndef Libccnx_config_TestingComponent_h
#define Libccnx_config_TestingComponent_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "TESTING UPPER" : { } }
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
CCNxStackConfig *testingUpper_ProtocolStackConfig(CCNxStackConfig *stackConfig);

/**
 * Generates the configuration settings included in the Protocol Stack configuration
 *
 * Adds configuration elements to the Protocol Stack configuration
 *
 * { "TESTING LOWER" : { } }
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
CCNxStackConfig *testingLower_ProtocolStackConfig(CCNxStackConfig *stackConfig);

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 * @param [in] config A pointer to a valid `CCNxConnectionConfig` instance.
 *
 * @return non-null A pointer to the modified `CCNxConnectionConfig` instance
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxConnectionConfig *testingUpper_ConnectionConfig(CCNxConnectionConfig *config);

/**
 * Generates the configuration settings included in the Connection configuration
 *
 * Adds configuration elements to the `CCNxConnectionConfig`
 *
 * @param [in] config A pointer to a valid `CCNxConnectionConfig` instance.
 *
 * @return non-null The modified `CCNxConnectionConfig`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxConnectionConfig *testingLower_ConnectionConfig(CCNxConnectionConfig *config);


/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *testingUpper_GetName(void);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *testingLower_GetName(void);
#endif // Libccnx_config_TestingComponent_h
