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
 * @file rta_CommandDestroyProtocolStack.h
 * @brief Represents a command to destroy a protocol stack
 *
 * Used to construct an RtaCommand object that is passed to rtaTransport_PassCommand() or _rtaTransport_SendCommandToFramework()
 * to send a command from the API's thread of execution to the Transport's thread of execution.
 *
 */
#ifndef Libccnx_rta_CommandDestroyProtocolStack_h
#define Libccnx_rta_CommandDestroyProtocolStack_h

struct rta_command_destroyprotocolstack;
typedef struct rta_command_destroyprotocolstack RtaCommandDestroyProtocolStack;

/**
 * Creates a DestroyProtocolStack command object
 *
 * Creates a DestroyProtocolStack command object used to signal the RTA framework to
 * destroy a protocol stack and all connections in it.
 *
 * @param [in] stackId The ID used to create the protocol stack.
 *
 * @return non-null An allocated object
 * @return null An error
 *
 * Example:
 * @code
 * void foo(RTATransport *transport)
 * {
 *     int stackId = nextStackIdNumber();
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *
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
 *     rtaCommandDestroyProtocolStack_Release(&destroyStack);
 *     ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
RtaCommandDestroyProtocolStack *rtaCommandDestroyProtocolStack_Create(int stackId);

/**
 * Increase the number of references to a `RtaCommandDestroyProtocolStack`.
 *
 * Note that new `RtaCommandDestroyProtocolStack` is not created,
 * only that the given `RtaCommandDestroyProtocolStack` reference count is incremented.
 * Discard the reference by invoking `rtaCommandDestroyProtocolStack_Release`.
 *
 * @param [in] destroyStack The RtaCommandDestroyProtocolStack to reference.
 *
 * @return non-null A reference to `destroyStack`.
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxStackConfig *config = ccnxStackConfig_Create();
 *    RtaCommandDestroyProtocolStack *destroyStack = rtaCommandCreateProtocolStack_Create(stackId, config);
 *    RtaCommandDestroyProtocolStack *second = rtaCommandDestroyProtocolStack_Acquire(destroyStack);
 *
 *    // release order does not matter
 *    rtaCommandDestroyProtocolStack_Release(&destroyStack);
 *    rtaCommandDestroyProtocolStack_Release(&second);
 *    ccnxStackConfig_Release(&config);
 * }
 * @endcode
 */
RtaCommandDestroyProtocolStack *rtaCommandDestroyProtocolStack_Acquire(const RtaCommandDestroyProtocolStack *destroyStack);

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
 *     RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);
 *     RtaCommand *command = rtaCommand_CreateDestroyProtocolStack(destroyStack);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandDestroyProtocolStack(&destroyStack);
 * }
 * @endcode
 */
void rtaCommandDestroyProtocolStack_Release(RtaCommandDestroyProtocolStack **destroyStackPtr);

/**
 * Returns the Stack ID of the destroy stack command
 *
 * Returns the Stack ID parameter.
 *
 * @param [in] destroyStack An allocated RtaCommandDestroyProtocolStack
 *
 * @return integer The value passed to rtaCommandDestroyProtocolStack_Create().
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *     RtaCommandDestroyProtocolStack *destroyStack = rtaCommandDestroyProtocolStack_Create(stackId);
 *     int testValue = rtaCommandDestroyProtocolStack_GetStackId(destroyStack);
 *     assertTrue(testValue == stackId, "Wrong value got %d expected %d", testValue, stackId);
 *     rtaCommandDestroyProtocolStack(&destroyStack);
 * }
 * @endcode
 */
int rtaCommandDestroyProtocolStack_GetStackId(const RtaCommandDestroyProtocolStack *destroyStack);
#endif // Libccnx_rta_CommandDestroyProtocolStack_h
