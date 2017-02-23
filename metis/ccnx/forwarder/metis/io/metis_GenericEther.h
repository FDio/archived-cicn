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
 * @file generic_ether.h
 * @brief Generic interface to working with Ethernet frames.
 *
 * Wraps platform-specific code.  The implementation is found in the metis/platforms directory.
 *
 */

#ifndef Metis_metis_GenericEther_h
#define Metis_metis_GenericEther_h

#include <stdbool.h>
#include <parc/algol/parc_EventBuffer.h>
#include <parc/algol/parc_Buffer.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_generic_ether;
typedef struct metis_generic_ether MetisGenericEther;

/**
 * Create a generic ethernet object
 *
 * Hides system dependent ethernet.  Creates an ethernet object that is ready to send and
 * receive ethernet frames on a given device and ether type.  There may be system limits
 * to the number of these you can open (i.e. 4 BPF devices on Mac OS).
 *
 * If the device name is NULL, it will not bind to a specific interface.
 *
 * You generally need elevated permissions to access an Ethernet device.  This function
 * may return NULL due to a permissions error.
 *
 * @param [in] deviceName The name of the device, e.g. "eth0" or "en1"
 * @param [in] etherType The host-byte-order ethertype (i.e. 0x0801)
 * @param [in] logger The MetisLogger to use
 *
 * @retval non-null An allocated ethernet object
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisGenericEther *metisGenericEther_Create(MetisForwarder *metis, const char *deviceName, uint16_t etherType);

/**
 * Acquire a reference counted copy
 *
 * Returns a reference counted copy of the generic Ethernet object
 *
 * @param [in] ether An allocated Ethernet object
 *
 * @retval non-null A reference counted copy
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisGenericEther *metisGenericEther_Acquire(const MetisGenericEther *ether);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisGenericEther_Release(MetisGenericEther **etherPtr);

/**
 * Returns the descriptor for i/o
 *
 * Returns a system descriptor that can be used to select, poll, etc.
 *
 * Do not close the socket.  It will be closed when metisGenericEther_Release() is called.
 *
 * @param [in] ether An allocated generic ethernet
 *
 * @retval non-negative The system descriptor
 * @retval negative An error (use errno)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int metisGenericEther_GetDescriptor(const MetisGenericEther *ether);


/**
 * Reads the next ethernet frame in to the provided buffer
 *
 * The frame will have the Ethernet header
 *
 * @param [in] ether An allocated generic ethernet
 * @param [in] buffer The allocated buffer to put the packet in
 *
 * @retval true A frame was ready and put in the buffer
 * @retval false No frame is ready
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisGenericEther_ReadNextFrame(MetisGenericEther *ether, PARCEventBuffer *buffer);

/**
 * Sends an Ethernet frame out the device
 *
 * The frame must have an Ethernet header filled in with all values.
 *
 * @param [in] ether An allocated GenericEther object
 * @param [in] buffer The buffer to send, including the Ethernet header
 *
 * @retval true The frame was sent
 * @retval false An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisGenericEther_SendFrame(MetisGenericEther *ether, PARCEventBuffer *buffer);

/**
 * Return the MAC address the object is bound to
 *
 * Returns a PARCBuffer with the 6-byte mac address of the interface
 *
 * @param [in] ether An allocated GenericEther object
 *
 * @retval non-null The MAC address of the interface
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *metisGenericEther_GetMacAddress(const MetisGenericEther *ether);

/**
 * Returns the ethertype associated with this object
 *
 * Returns the ethertype, in host byte order.
 *
 * @param [in] ether An allocated GenericEther object
 *
 * @retval number Ethertype (host byte order)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
uint16_t metisGenericEther_GetEtherType(const MetisGenericEther *ether);

/**
 * Returns the maximum transmission unit (MTU)
 *
 * The MTU is the largest user payload allowed in a frame
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
unsigned metisGenericEther_GetMTU(const MetisGenericEther *ether);
#endif // Metis_metis_GenericEther_h
