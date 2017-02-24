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
 * @file metis_EtherConnection.h
 * @brief Represents an ethernet pair (source address, destination address) in the connection table
 *
 * Ethernet connections are never local.
 *
 */

#ifndef Metis_metis_EtherConnection_h
#define Metis_metis_EtherConnection_h

#include <ccnx/forwarder/metis/io/metis_IoOperations.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/io/metis_AddressPair.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/forwarder/metis/io/metis_GenericEther.h>
#include <ccnx/forwarder/metis/io/metis_HopByHopFragmenter.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>

/**
 * @function metisEtherConnection_Create
 * @abstract <#OneLineDescription#>
 * @discussion
 *   <#Discussion#>
 *
 * @param pair the address pair that uniquely identifies the connection.  Takes ownership of this memory.
 * @return <#return#>
 */
MetisIoOperations *metisEtherConnection_Create(MetisForwarder *metis, MetisGenericEther *ether, MetisAddressPair *pair);

/**
 * If the IO Operats are of type MetisEtherConnection, return its fragmenter
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] conn An allocated MetisConnection
 *
 * @return non-null The fragmenter associated with this conneciton
 * @return null There is no such fragmenter or the ops is not a MetisEtherConnection
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
MetisHopByHopFragmenter *metisEtherConnection_GetFragmenter(const MetisConnection *conn);

/**
 * Tests if MetisEtherConnection is the underlying implementation of the connection
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] conn An allocated MetisConnection
 *
 * @return true The connection is of type MetisEtherConnection
 * @return false The connection is of other type
 *
 * Example:
 * @code
 * {
 *    MetisHopByHopFragmenter *
 *    metisEtherConnection_GetFragmenter(const MetisConnection *conn)
 *    {
 *        MetisHopByHopFragmenter *fragmenter = NULL;
 *
 *        if (metisEtherConnection_IsInstanceOf(conn)) {
 *            MetisIoOperations *ops = metisConnection_GetIoOperations(conn);
 *            _MetisEtherState *state = (_MetisEtherState *) ops->context;
 *            fragmenter = state->fragmenter;
 *        }
 *        return fragmenter;
 *    }
 * }
 * @endcode
 */
bool metisEtherConnection_IsInstanceOf(const MetisConnection *conn);

#endif // Metis_metis_EtherConnection_h
