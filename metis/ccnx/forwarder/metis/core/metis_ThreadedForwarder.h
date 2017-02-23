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
 * @header Metis Threaded Forwarder
 * @abstract This is a wrapper around metis_Forwarder to run it as a thread
 * @discussion
 *     <#Discussion#>
 *
 */

#ifndef Metis_metis_ThreadedForwarder_h
#define Metis_metis_ThreadedForwarder_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_threaded_forwarder;
typedef struct metis_threaded_forwarder MetisThreadedForwarder;

/**
 * @function metisThreadedForwarder_Create
 * @abstract Creates a threaded forwarder in the stopped state
 * @discussion
 *   IMPORTANT: The logger is called from the Metis thread, so it is up to
 *   the user to implement any necessary thread saftey in the logger.  There
 *   is only a single metis thread, so it does not need to be re-enterent.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisThreadedForwarder *metisThreadedForwarder_Create(MetisLogger *logger);

/**
 * @function metisThreadedForwarder_AddCLI
 * @abstract Add a command line interface (CLI) on the given port
 * @discussion
 *   MUST BE DONE PRIOR TO START.  This function will add a CLI to the forwarder
 *   prior to starting it.  Once started, will assert if you try to do this.
 *
 * @param <#param1#>
 */
void metisThreadedForwarder_AddCLI(MetisThreadedForwarder *metis, uint16_t port);

/**
 * @function metisThreadedForwarder_AddTcpListener
 * @abstract Adds a TCP listenener
 * @discussion
 *   MUST BE DONE PRIOR TO START.
 *   May be IPv4 or IPv6
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisThreadedForwarder_AddTcpListener(MetisThreadedForwarder *metis, struct sockaddr *address);

/**
 * @function metisThreadedForwarder_SetupAllListeners
 * @abstract Setup all tcp/udp ipv4/ipv6 listeners on the given port
 * @discussion
 *   MUST BE DONE PRIOR TO START.
 *
 * @param port is the UDP and TCP port
 * @param localPath is the AF_UNIX path, may be NULL for no AF_UNIX socket.
 * @return <#return#>
 */
void metisThreadedForwarder_SetupAllListeners(MetisThreadedForwarder *metis, uint16_t port, const char *localPath);

/**
 * @function metisThreadedForwarder_Start
 * @abstract Blocks until started
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisThreadedForwarder_Start(MetisThreadedForwarder *metis);

/**
 * @function metisThreadedForwarder_Stop
 * @abstract Blocks until stopped
 * @discussion
 *   Currently we do not support re-starting a thread after it is stopped.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisThreadedForwarder_Stop(MetisThreadedForwarder *metis);

/**
 * @function metisThreadedForwarder_Destroy
 * @abstract Blocks until stopped and destoryed
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisThreadedForwarder_Destroy(MetisThreadedForwarder **metisPtr);
#endif // Metis_metis_ThreadedForwarder_h
