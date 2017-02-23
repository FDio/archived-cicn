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
 * @file metis_Connection.h
 * @brief Wrapper for different types of connections
 *
 * A connection wraps a specific set of {@link MetisIoOperations}.  Those operations
 * allow for input and output.  Connections get stored in the Connection Table.
 *
 */

#ifndef Metis_metis_Connection_h
#define Metis_metis_Connection_h
#include <config.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/io/metis_IoOperations.h>

//packet types for probing
#define METIS_PACKET_TYPE_PROBE_REQUEST 5
#define METIS_PACKET_TYPE_PROBE_REPLY 6

struct metis_connection;
typedef struct metis_connection MetisConnection;


/**
 * Creates a connection object.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisConnection *metisConnection_Create(MetisIoOperations *ops);

/**
 * @function metisConnection_Release
 * @abstract Releases a reference count, destroying on last release
 * @discussion
 *   Only frees the memory on the final reference count.  The pointer will
 *   always be NULL'd.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisConnection_Release(MetisConnection **connectionPtr);

/**
 * @function metisConnection_Acquire
 * @abstract A reference counted copy.
 * @discussion
 *   A shallow copy, they share the same memory.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisConnection *metisConnection_Acquire(MetisConnection *connection);

/**
 * @function metisConnection_Send
 * @abstract Sends the ccnx message on the connection
 * @discussion
 *
 * @param <#param1#>
 * @return true if message sent, false if connection not up
 */
bool metisConnection_Send(const MetisConnection *conn, MetisMessage *message);

/**
 * Return the `MetisIoOperations` instance associated with the specified `MetisConnection` instance.
 *
 * @param [in] connection The allocated connection
 *
 * @return a pointer to the MetisIoOperations instance associated by th specified connection.
 *
 * Example:
 * @code
 * {
 *     MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
 * }
 * @endcode
 */
MetisIoOperations *metisConnection_GetIoOperations(const MetisConnection *conn);

/**
 * Returns the unique identifier of the connection
 *
 * Calls the underlying MetisIoOperations to fetch the connection id
 *
 * @param [in] connection The allocated connection
 *
 * @return unsigned The unique connection id
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
unsigned metisConnection_GetConnectionId(const MetisConnection *conn);

/**
 * Returns the (remote, local) address pair that describes the connection
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] connection The allocated connection
 *
 * @return non-null The connection's remote and local address
 * @return null Should never return NULL
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const MetisAddressPair *metisConnection_GetAddressPair(const MetisConnection *conn);

/**
 * Tests if the connection is in the "up" state
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] connection The allocated connection
 *
 * @return true The connection is in the "up" state
 * @return false The connection is not in the "up" state
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisConnection_IsUp(const MetisConnection *conn);

/**
 * Tests if the connection is to a Local/Loopback address
 *
 * A local connection is PF_LOCAL (PF_UNIX) and a loopback connection is
 * 127.0.0.0/8 or ::1 for IPv6.
 *
 * @param [in] connection The allocated connection
 *
 * @retval true The connection is local or loopback
 * @retval false The connection is not local or loopback
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisConnection_IsLocal(const MetisConnection *conn);

/**
 * Returns an opaque pointer representing the class of the Io Operations
 *
 * Returns an opaque pointer that an implementation can use to detect if
 * the connection is based on that class.
 *
 * @param [in] conn The MetisConnection to test
 *
 * @return non-null An opaque pointer for each concrete implementation
 *
 * Example:
 * @code
 * {
 *    bool
 *    metisEtherConnection_IsClass(const MetisConnection *conn)
 *    {
 *        bool result = false;
 *        const void *class = metisConnection_Class(conn);
 *        if (class == _metisIoOperationsGuid) {
 *            result = true;
 *        }
 *        return result;
 *    }
 *
 *    MetisHopByHopFragmenter *
 *    metisEtherConnection_GetFragmenter(const MetisConnection *conn)
 *    {
 *        MetisHopByHopFragmenter *fragmenter = NULL;
 *
 *        if (metisEtherConnection_IsClass(conn)) {
 *            MetisIoOperations *ops = metisConnection_GetIoOperations(conn);
 *            _MetisEtherState *state = (_MetisEtherState *) ops->context;
 *            fragmenter = state->fragmenter;
 *        }
 *        return fragmenter;
 *    }
 * }
 * @endcode
 */
const void *metisConnection_Class(const MetisConnection *conn);

bool metisConnection_ReSend(const MetisConnection *conn, MetisMessage *message);

void metisConnection_Probe(MetisConnection *conn);
void metisConnection_HandleProbe(MetisConnection *conn, uint8_t *message, MetisTicks actualTime);
uint64_t metisConnection_GetDelay(MetisConnection *conn);
void metisConnection_EnableWldr(MetisConnection *conn);
void metisConnection_DisableWldr(MetisConnection *conn);
bool metisConnection_HasWldr(const MetisConnection *conn);
void metisConnection_DetectLosses(MetisConnection *conn, MetisMessage *message);

#endif // Metis_metis_Connection_h
