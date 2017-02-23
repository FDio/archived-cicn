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
 * @file parc_EventSocket.h
 * @ingroup events
 * @brief Socket events
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_EventSocket_h
#define libparc_parc_EventSocket_h

#include <sys/types.h>
#include <sys/socket.h>

/**
 * Current implementation based on top of libevent2
 */

#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_Event.h>

typedef struct PARCEventSocket PARCEventSocket;

typedef void (PARCEventSocket_Callback)(int fd, struct sockaddr *address,
                                        int socklen, void *user_data);
typedef void (PARCEventSocket_ErrorCallback)(PARCEventScheduler *,
                                             int error, char *errorString,
                                             void *user_data);

/**
 * Create a socket event handler instance.
 *
 * The event instance is passed in.
 *
 * @param [in] parcEventScheduler the scheduler instance
 * @param [in] callback the callback function.
 * @param [in] errorCallback the error callback function.
 * @param [in] userData pointer to private arguments for instance callback function
 * @param [in] sa is the socket address to bind to (INET, INET6, LOCAL)
 * @param [in] socklen is the sizeof the actual sockaddr (e.g. sizeof(sockaddr_un))
 * @returns A pointer to a new PARCEventSocket instance.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 *
 */
PARCEventSocket *parcEventSocket_Create(PARCEventScheduler *parcEventScheduler,
                                        PARCEventSocket_Callback *callback,
                                        PARCEventSocket_ErrorCallback *errorCallback,
                                        void *userData,
                                        const struct sockaddr *sa, int socklen);

/**
 * Destroy a socket event handler instance.
 *
 * The event instance is passed in.
 *
 * @param [in] parcEventSocket the address of the instance to destroy.
 *
 * Example:
 * @code
 * {
 *     PARCEvent *event = parcEventSocket_Destroy(&parcEventSocket);
 * }
 * @endcode
 *
 */
void parcEventSocket_Destroy(PARCEventSocket **parcEventSocket);

/**
 * Turn on debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventSocket_EnableDebug();
 * }
 * @endcode
 *
 */
void parcEventSocket_EnableDebug(void);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventSocket_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEventSocket_DisableDebug(void);
#endif // libparc_parc_EventSocket_h
