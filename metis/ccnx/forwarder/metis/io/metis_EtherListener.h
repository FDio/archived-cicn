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
 * @file metis_EtherListener.h
 * @brief Listen for raw ethernet frames on an interface
 *
 * <#Detailed Description#>
 *
 */

#ifndef Metis_metis_EtherListener_h
#define Metis_metis_EtherListener_h

#include <ccnx/forwarder/metis/io/metis_GenericEther.h>

struct metis_listener_ether;
typedef struct metis_listener_ether MetisListenerEther;

/**
 * @function metisListenerEther_Create
 * @abstract Create a L2 listener on a raw ethertype
 * @discussion
 *   Requires root, will send/receive ethernet frames on the specified device.
 *   The exact mechanism varies by system.
 *
 * @param deviceName is the system name of the interface (e.g. "en0")
 * @return <#return#>
 */
MetisListenerOps *metisEtherListener_Create(MetisForwarder *metis, const char *deviceName, uint16_t ethertype);

/**
 * Return the underlying GenericEther of the listener
 *
 * The MetisGenericEther wraps the platform-specific IO operations of the ethernet connection.
 * Will assert if the listenerOps is not of type METIS_ENCAP_ETHER.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval non-null The underlying generic ethernet
 * @retval null An error
 *
 * Example:
 * @code
 * {
 *     MetisListenerSet *listenerSet = metisForwarder_GetListenerSet(metis);
 *     MetisListenerOps *listenerOps = metisListenerSet_Find(listenerSet, METIS_ENCAP_ETHER, linkAddress);
 *     if (listenerOps) {
 *         MetisGenericEther *ether = metisEtherListener_GetGenericEtherFromListener(listenerOps);
 *     }
 * }
 * @endcode
 */
MetisGenericEther *metisEtherListener_GetGenericEtherFromListener(MetisListenerOps *listenerOps);
#endif // Metis_metis_EtherListener_h
