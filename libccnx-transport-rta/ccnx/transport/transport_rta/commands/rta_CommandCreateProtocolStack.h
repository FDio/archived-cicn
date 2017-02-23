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
 * @file rta_CommandCreateProtocolStack.h
 * @brief Represents a command to create a protocol stack
 *
 * Used to construct an RtaCommand object that is passed to rtaTransport_PassCommand() or _rtaTransport_SendCommandToFramework()
 * to send a command from the API's thread of execution to the Transport's thread of execution.
 *
 */
#ifndef Libccnx_rta_CommandCreateProtocolStack_h
#define Libccnx_rta_CommandCreateProtocolStack_h

struct rta_command_createprotocolstack;
typedef struct rta_command_createprotocolstack RtaCommandCreateProtocolStack;


#include <ccnx/transport/common/ccnx_StackConfig.h>

/**
 * Creates a CreateProtocolStack command object
 *
 * Creates a CreateProtocolStack command object used to signal the RTA framework to
 * create a new Protocol Stack with the specified stackId and configuration.  The caller is
 * responsible for ensuring that the stackId is unique among existing stacks (the framework might
 * assert an error for duplicates).  Note that the check for a unique stack ID is only done
 * once the RtaCommandCreateProtocolStack is passed to the RtaFramework, not on creation
 * of this object.
 *
 * @param [in] stackId The new (unique) ID for the stack to create
 * @param [in] config the JSON representation of the stack configuration
 *
 * @return non-null An allocated object
 * @return null An error
 *
 * Example:
 * @code
 * void
 * foo(RTATransport *transport)
 * {
 *     int stackId = nextStackIdNumber();
 *
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *     RtaCommand *command = rtaCommand_CreateCreateProtocolStack(createStack);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *
 *     // ... do work ...
 *
 *     RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);
 *     command = rtaCommand_CreateDestroyProtocolStack(destroyStack);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandDestroyProtocolStack(&destroyStack);
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
RtaCommandCreateProtocolStack *rtaCommandCreateProtocolStack_Create(int stackId, CCNxStackConfig *config);

/**
 * Increase the number of references to a `RtaCommandCreateProtocolStack`.
 *
 * Note that new `RtaCommandCreateProtocolStack` is not created,
 * only that the given `RtaCommandCreateProtocolStack` reference count is incremented.
 * Discard the reference by invoking `rtaCommandCreateProtocolStack_Release`.
 *
 * @param [in] createStack The RtaCommandCreateProtocolStack to reference.
 *
 * @return non-null A reference to `createStack`.
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *     RtaCommandCreateProtocolStack *second = rtaCommandCreateProtocolStack_Acquire(createStack);
 *
 *     // release order does not matter
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *     rtaCommandCreateProtocolStack_Release(&second);
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
RtaCommandCreateProtocolStack *rtaCommandCreateProtocolStack_Acquire(const RtaCommandCreateProtocolStack *createStack);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] closePtr A pointer to the object to release, will return NULL'd.
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *     RtaCommand *command = rtaCommand_CreateCreateProtocolStack(createStack);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
void rtaCommandCreateProtocolStack_Release(RtaCommandCreateProtocolStack **createStackPtr);

/**
 * Returns the Stack ID of the create stack command
 *
 * Returns the Stack ID parameter.
 *
 * @param [in] createStack An allocated RtaCommandCreateProtocolStack
 *
 * @return integer The value passed to rtaCommandCreateProtocolStack_Create().
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *     int testValue = rtaCommandCreateProtocolStack_GetStackId(createStack);
 *     assertTrue(testValue == stackId, "Wrong value got %d expected %d", testValue, stackId);
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
int rtaCommandCreateProtocolStack_GetStackId(const RtaCommandCreateProtocolStack *createStack);

/**
 * Get the CCNxStackConfig used by the given `RtaCommandCreateProtocolStack` instance.
 *
 * @param [in] createStack A pointer to a valid `RtaCommandCreateProtocolStack` instance.
 *
 * @return A pointer to the CCNxStackConfig used by the given `RtaCommandCreateProtocolStack` instance.
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *
 *     CCNxStackConfig *config = rtaCommandCreateProtocolStack_GetStackConfig(createStack);
 *
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
CCNxStackConfig *rtaCommandCreateProtocolStack_GetStackConfig(const RtaCommandCreateProtocolStack *createStack);

/**
 * Returns the PARCJSON stack configuration of the create stack command
 *
 * Returns the JSON representation of the stack configuration.
 *
 * @param [in] createStack An allocated RtaCommandCreateProtocolStack
 *
 * @return The value passed to rtaCommandCreateProtocolStack_Create().
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *     RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *
 *     PARCJSON *testValue = rtaCommandCreateProtocolStack_GetConfig(createStack);
 *     assertTrue(ccnxJson_Equals(config, testValue), "Wrong value");
 *     rtaCommandCreateProtocolStack_Release(&createStack);
 *
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
PARCJSON *rtaCommandCreateProtocolStack_GetConfig(const RtaCommandCreateProtocolStack *createStack);

/**
 * Derive an explanation for why a RtaCommandCreateProtocolStack instance is invalid.
 *
 * Returns either a nul-terminated C string containing a human-readable explanation,
 * or NULL which indicates the instance is valid.
 *
 * @param [in] instance A pointer to a `RtaCommandCreateProtocolStack` instance.
 *
 * @return NULL The instance is valid.
 * @return non-NULL A nul-terminated C string containing an explanation.
 *
 * Example:
 * @code
 * {
 *     RtaCommandCreateProtocolStack *instance = rtaCommandCreateProtocolStack_Create(...);
 *
 *     if (rtaCommandCreateProtocolStack_IsValid(instance)) {
 *         printf("Instance is valid.\n");
 *     }
 * }
 * @endcode
 */
const char *rtaCommandCreateProtocolStack_AssessValidity(const RtaCommandCreateProtocolStack *instance);

/**
 * Determine if an instance of `RtaCommandCreateProtocolStack` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a `RtaCommandCreateProtocolStack` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     RtaCommandCreateProtocolStack *instance = rtaCommandCreateProtocolStack_Create(...);
 *
 *     if (rtaCommandCreateProtocolStack_IsValid(instance)) {
 *         printf("Instance is valid.\n");
 *     }
 * }
 * @endcode
 */
bool rtaCommandCreateProtocolStack_IsValid(const RtaCommandCreateProtocolStack *instance);

/**
 * Assert that the given `RtaCommandCreateProtocolStack` instance is valid.
 *
 * @param [in] instance A pointer to a valid RtaCommandCreateProtocolStack instance.
 *
 * Example:
 * @code
 * {
 *     RtaCommandCreateProtocolStack *a = rtaCommandCreateProtocolStack_Create();
 *
 *     rtaCommandCreateProtocolStack_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     rtaCommandCreateProtocolStack_Release(&b);
 * }
 * @endcode
 */
void rtaCommandCreateProtocolStack_AssertValid(const RtaCommandCreateProtocolStack *instance);
#endif // Libccnx_rta_CommandCreateProtocolStack_h
