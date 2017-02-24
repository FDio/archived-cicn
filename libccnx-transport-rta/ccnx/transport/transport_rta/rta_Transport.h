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


#include <stdint.h>
#include <ccnx/transport/common/transport.h>

/**
 * rtaTransport executes in the API's thread.  It glues the bottom half of
 * the Transport API to the RTA transport.  It owns and manages a worker thread
 * in which the event schduler executes.
 *
 * rtaTransport is thread safe.  You may have multiple threads using the same
 * transport context.
 *
 * Inside the worker thread, the event scheduler executes without locks.  Therefore we need
 * to message pass to it and have it execute our commands in a managed callback.
 * This is done by passing commands (JSON) over a socket pair.
 *
 * Inside the worker thread, rta_Framework provides service utilities to components
 * and connectors.  It also manages the command socket.
 *
 * When an API calls <code>int rtaTransport_Open(CCNxJSON *params)</code>, rtaTransport
 * will create a socket pair and give one back to the api (api_fd) and send one to
 * rtaFramework (transport_fd).
 *
 * The socket commands are (in JSON):
 *
 * PARAMS := existing SYSTEM and USER JSON objects, i.e.:
 * { "SYSTEM" : {...}, "USER" : {...} }
 *
 * { "RTA" : { "CREATE STACK" : stack_id, PARAMS }
 * { "RTA" : { "OPEN" : [stack_id, api_fd, transport_fd], PARAMS } }
 * { "RTA" : { "CLOSE": transport_fd } }
 * { "RTA" : { "DESTROY STACK": stack_id } }
 * { "RTA" : { "SHUTDOWN" }
 *
 * See rta_Commands.h for an implementation of this.
 */
#ifndef Libccnx_rta_Transport_h
#define Libccnx_rta_Transport_h

#include <ccnx/transport/common/transport.h>
#include <ccnx/transport/transport_rta/commands/rta_Command.h>

/**
 * Transport Ready To Assemble context
 *
 */
struct rta_transport;
typedef struct rta_transport RTATransport;

/**
 * Structure of function points to operate on Transport RTA
 *
 */
extern const struct transport_operations rta_ops;

/**
 * Create the transport.  No locks here, as rtaFramework_Create and rtaFramework_Start
 * are thread-safe functions and we dont maintain any data.
 *
 */
RTATransport *rtaTransport_Create(void);

int rtaTransport_Destroy(RTATransport **ctxPtr);

int rtaTransport_Open(RTATransport *ctx, CCNxTransportConfig *transportConfig);

/**
 * Send a CCNxMetaMessage on the outbound direction of the stack.
 *
 * @param [in] transport A pointer to a valid RTATransport instance.
 * @param [in] queueId The identifier of the asynchronous queue between the top and bottom halves of the stack.
 * @param [in] message A pointer to a valid CCNxMetaMessage instance.
 *
 * @return true The send was successful
 * @return false The send was not successful
 */
//bool rtaTransport_Send(RTATransport *transport, int queueId, const CCNxMetaMessage *message, const struct timeval *restrict timeout);
bool rtaTransport_Send(RTATransport *transport, int queueId, const CCNxMetaMessage *message, const uint64_t *microSeconds);

TransportIOStatus rtaTransport_Recv(RTATransport *transport, const int queueId, CCNxMetaMessage **msgPtr, const uint64_t *microSeconds);

int rtaTransport_Close(RTATransport *transport, int desc);

int rtaTransport_PassCommand(RTATransport *transport, const RtaCommand *rtacommand);

#endif // Libccnx_rta_Transport_h
