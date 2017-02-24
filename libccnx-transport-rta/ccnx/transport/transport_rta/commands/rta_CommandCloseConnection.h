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
 * @file rta_CommandCloseConnection.h
 * @brief Represents a command to close a connection
 *
 * Used to construct an RtaCommand object that is passed to rtaTransport_PassCommand() or _rtaTransport_SendCommandToFramework()
 * to send a command from the API's thread of execution to the Transport's thread of execution.
 *
 */
#ifndef Libccnx_rta_CommandCloseConnection_h
#define Libccnx_rta_CommandCloseConnection_h

struct rta_command_closeconnection;
typedef struct rta_command_closeconnection RtaCommandCloseConnection;

/**
 * Creates a CloseConnection command object
 *
 * Creates a CloseConnection command object used to signal the RTA framework to
 * close a specified connection.  The user passes its socket number to the close
 * command to signify which connection.
 *
 * The apiNotifierFd number must correspond to the apiSocket number used in rtaCommandOpenConnection().
 *
 * @param [in] apiNotifierFd The descriptor number used by the API
 *
 * @return non-null An allocated object
 * @return null An error
 *
 * Example:
 * @code
 * void foo(RTATransport *transport)
 * {
 *     #define API_SIDE 0
 *     #define TRANSPORT_SIDE 1
 *
 *     int pair[2];
 *     socketpair(AF_LOCAL, SOCK_STREAM, 0, pair);
 *     PARCJSON *config = createConnectionConfig();
 *
 *     RtaCommandOpenConnection *openCommand = rtaCommandOpenConnection_Create(6, pair[API_SIDE], pair[TRANSPORT_SIDE], config);
 *     RtaCommand *command = rtaCommand_CreateOpenConnection(openCommand);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandOpenConnection_Release(&openCommand);
 *
 *     // ... do work ...
 *
 *     RtaCommandCloseConnection *closeCommand = rtaCommandCloseConnection_Create(pair[API_SIDE]);
 *     command = rtaCommand_CreateCloseConnection(openCommand);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandCloseConnection_Release(&closeCommand);
 * }
 * @endcode
 */
RtaCommandCloseConnection *rtaCommandCloseConnection_Create(int apiNotifierFd);

/**
 * Increase the number of references to a `RtaCommandCloseConnection`.
 *
 * Note that new `RtaCommandCloseConnection` is not created,
 * only that the given `RtaCommandCloseConnection` reference count is incremented.
 * Discard the reference by invoking `rtaCommandCloseConnection_Release`.
 *
 * @param [in] closeConnection The RtaCommandCloseConnection to reference.
 *
 * @return non-null A reference to `closeConnection`.
 * @return null An error
 *
 * Example:
 * @code
 * {
 *    RtaCommandCloseConnection *closeConnection = rtaCommandCloseConnection_Create(pair[API_SIDE]);
 *    RtaCommandCloseConnection *second = rtaCommandCloseConnection_Acquire(closeConnection);
 *
 *    // release order does not matter
 *    rtaCommandCloseConnection_Release(&closeConnection);
 *    rtaCommandCloseConnection_Release(&second);
 * }
 * @endcode
 */
RtaCommandCloseConnection *rtaCommandCloseConnection_Acquire(const RtaCommandCloseConnection *closeConnection);

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
 *     RtaCommandCloseConnection *closeCommand = rtaCommandCloseConnection_Create(pair[API_SIDE]);
 *     RtaCommand *command = rtaCommand_CreateCloseConnection(openCommand);
 *     _rtaTransport_SendCommandToFramework(transport, command);
 *     rtaCommand_Release(&command);
 *     rtaCommandCloseConnection_Release(&closeCommand);
 * }
 * @endcode
 */
void rtaCommandCloseConnection_Release(RtaCommandCloseConnection **closePtr);

/**
 * Returns the API notifier descriptor of the close command
 *
 * Returns the apiNotifierFd parameter.
 *
 * @param [in] closeConnection An allocated RtaCommandCloseConnection
 *
 * @return integer The value passed to rtaCommandCloseConnection_Create().
 *
 * Example:
 * @code
 * {
 *     int apiNotifierFd = 7;
 *     RtaCommandCloseConnection *closeCommand = rtaCommandCloseConnection_Create(apiNotifierFd);
 *     int testValue = rtaCommandCloseConnection_GetApiNotifierFd(closeCommand);
 *     assertTrue(testValue == apiNotifierFd, "Wrong value got %d expected %d", testValue, apiNotifierFd);
 *     rtaCommandCloseConnection_Release(&closeCommand);
 * }
 * @endcode
 */
int rtaCommandCloseConnection_GetApiNotifierFd(const RtaCommandCloseConnection *closeConnection);
#endif // Libccnx_rta_CommandCloseConnection_h
