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
 * @file ccnx_TransportConfig.h
 * @brief The Transport Configuration information.
 *
 * The API composes the stack and connection parameters using these functions.
 * The examples below are for the RTA Transport.
 *
 * <code>
 * CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
 * ccnxStackConfig_AppendComponents(stackConfig, { RtaComponentNames[API_CONNECTOR], RtaComponentNames[FC_VEGAS],
 * RtaComponentNames[CODEC_CCNB], RtaComponentNames[FWD_CCND] } );
 *
 * ccnxStackConfig_AppendApiConnector(stackConfig);
 * ccnxStackConfig_AppendVegasFlowController(stackConfig);
 * ccnxStackConfig_AppendCcndCodec(stackConfig);
 * ccnxStackConfig_AppendCcndForwarder(stackConfig);
 *
 * CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
 * ccnxConnectionConfig_publicKeySignerPkcs12Store(connConfig, "/Users/mmosko/keystore.p12", "123abc");
 * ccnxConnectionConfig_InMemoryVerifier(connConfig);
 *
 *
 * RtaCommand *cmdCreateStack = rtaCommand_CreateStack( (CommandCreateStack) { .stack_id = 7, .params = ccnxStackConfig_GetJson(stackConfig) } );
 * rtaCommand_write(cmdCreateStack, command_fd);
 * ccnxStackConfig_Release(&stackConfig);
 *
 * RtaCommand *cmdOpen = rtaCommand_Open( (CommandOpen) { .stack_id = 7, .api_fd = 12, .transport_fd = 13, .params = connecitonConfig_GetJson(connConfig) } );
 * rtaCommand_write(cmdCreateStack, command_fd);
 * ccnxConnectionConfig_Destroy(&connConfig);
 * </code>
 *
 */
#ifndef Libccnx_transport_Configuration_h
#define Libccnx_transport_Configuration_h

#include <stdarg.h>

#include <ccnx/transport/common/ccnx_StackConfig.h>
#include <ccnx/transport/common/ccnx_ConnectionConfig.h>

struct transport_config;
typedef struct transport_config CCNxTransportConfig;

/**
 * Create a `CCNxTransportConfig` instance.
 *
 * The instance must be populated with configuration information before it can be used.
 *
 * @param [in] stackConfig A pointer to a valid `CCNxStackConfig` instance.
 * @param [in] connectionConfig A pointer to a valid `CCNxConnectionConfig` instance.
 * @return NULL An error occurred.
 * @return non-NULL A valid `CCNxTransportConfig` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *
 *     ccnxTransportConfig_Destroy(&config);
 * @endcode
 */
CCNxTransportConfig *ccnxTransportConfig_Create(CCNxStackConfig *stackConfig, CCNxConnectionConfig *connectionConfig);

#ifdef CCNxTransport_DISABLE_VALIDATION
#  define ccnxTransportConfig_OptionalAssertValid(_instance_)
#else
#  define ccnxTransportConfig_OptionalAssertValid(_instance_) ccnxTransportConfig_AssertValid(_instance_)
#endif
/**
 * Assert that an instance of `CCNxTransportConfig` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] config A pointer to a `CCNxTransportConfig` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *     ccnxTransportConfig_AssertValid(config);
 *     ccnxTransportConfig_Destroy(&config);
 * }
 * @endcode
 * @see ccnxTransportConfig_IsValid
 */
void ccnxTransportConfig_AssertValid(const CCNxTransportConfig *config);

/**
 * Destroy previously created `CCNxTransportConfig` instance.
 *
 * @param [in] configPtr A pointer to a pointer to a valid `CCNxTransportConfig` instance that will be set to zero upon return.
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *
 *     ccnxTransportConfig_Destroy(&config);
 * }
 * @endcode
 */
void ccnxTransportConfig_Destroy(CCNxTransportConfig **configPtr);

/**
 * Get the `CCNxStackConfig` instance in the given `CCNxTransportConfig`
 *
 * @param [in] config A pointer to a valid `CCNxTransportConfig` instance.
 *
 * @return A pointer to the `CCNxStackConfig` instance in the given `CCNxTransportConfig`
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *
 *     CCNxStackConfig *stack = ccnxTransportConfig_GetStackConfig(config);
 *
 *     ccnxTransportConfig_Destroy(&config);
 * }
 * @endcode
 */
CCNxStackConfig *ccnxTransportConfig_GetStackConfig(const CCNxTransportConfig *config);

/**
 * Get the `CCNxConnectionConfig` instance in the given `CCNxTransportConfig`
 *
 * @param [in] config A pointer to a valid `CCNxTransportConfig` instance.
 *
 * @return A pointer to the `CCNxConnectionConfig` instance in the given `CCNxTransportConfig`
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *
 *     CCNxConnectionConfig *connection = ccnxTransportConfig_GetConnectionConfig(config);
 *
 *     ccnxTransportConfig_Destroy(&config);
 * }
 * @endcode
 */
CCNxConnectionConfig *ccnxTransportConfig_GetConnectionConfig(const CCNxTransportConfig *config);

/**
 * Determine if an instance of `CCNxTransportConfig` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] config A pointer to a `CCNxTransportConfig` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *config = ccnxTransportConfig_Create(stackConfig, connectionConfig);
 *     ccnxTransportConfig_IsValid(config);
 *     ccnxTransportConfig_Destroy(&config);
 * }
 * @endcode
 * @see ccnxTransportConfig_AssertValid
 */
bool ccnxTransportConfig_IsValid(const CCNxTransportConfig *config);
/**
 * Determine if two `CCNxTransportConfig` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxTransportConfig` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `ccnxTransportConfig_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxTransportConfig_Equals(x, y)` must return true if and only if
 *        `ccnxTransportConfig_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxTransportConfig_Equals(x, y)` returns true and
 *        `ccnxTransportConfig_Equals(y, z)` returns true,
 *        then `ccnxTransportConfig_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxTransportConfig_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxTransportConfig_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid CCNxTransportConfig instance.
 * @param [in] y A pointer to a valid CCNxTransportConfig instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxTransportConfig *a = ccnxTransportConfig_Create();
 *     CCNxTransportConfig *b = ccnxTransportConfig_Create();
 *
 *     if (ccnxTransportConfig_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     ccnxTransportConfig_Release(&a);
 *     ccnxTransportConfig_Release(&b);
 * }
 * @endcode
 * @see ccnxTransportConfig_HashCode
 */
bool ccnxTransportConfig_Equals(const CCNxTransportConfig *x, const CCNxTransportConfig *y);

/**
 * Make a copy of the given TransportConfig.  The original and copy
 * must both be destroyed.
 */
CCNxTransportConfig *ccnxTransportConfig_Copy(const CCNxTransportConfig *original);
#endif // Libccnx_transport_Configuration_h
