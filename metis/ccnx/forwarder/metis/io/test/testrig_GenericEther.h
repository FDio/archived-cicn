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
 * @file testrig_GenericEther.h
 * @brief A mockup of a platform Ethernet
 *
 * The mockup is connected to a socketpair, so you can read frames that the metis_EtherListener sends.
 * It also has an input queue so you can queue frames to be read by metis_EtherListener.
 *
 * This mockup implements the metis_GenericEther.h API plus two additional functions for the mockup.
 *
 */

#ifndef Metis_testrig_GenericEther_h
#define Metis_testrig_GenericEther_h

#include <ccnx/forwarder/metis/io/metis_GenericEther.h>
#include <parc/algol/parc_Buffer.h>

/**
 * Returns the other end of a socketpair that mocks up the ethernet wire
 *
 * The mockup does not connect to a RAW or BPF socket, it connects to a socketpair.
 * This function gets the remote end of the socket pair, which is where you can read
 * frames that you send.
 *
 * DO NOT WRITE PACKETS HERE.  To queue packets for input, use mockGenericEther_QueueFrame().
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval number System socketpair
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int mockGenericEther_GetTestDescriptor(MetisGenericEther *ether);

/**
 * Queue an Ethernet frame to be read
 *
 * The mockup maintains an input queue (deque) for input frames.  These should be full
 * Ethernet frames (not including the frame check sequence).
 *
 * This stores a reference, so caller must also release the PARCBuffer.
 *
 * This function will not notify the etherSocket being watched by Libevent in Metis.
 * To notify Libevent, use mockGenericEther_Notify() after queuing packets.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void mockGenericEther_QueueFrame(MetisGenericEther *ether, PARCBuffer *ethernetFrame);

/**
 * Writes a byte to the etherSocket
 *
 * Tickles Libevent by writing a byte to the etherSocket.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void mockGenericEther_Notify(MetisGenericEther *ether);

/**
 * Convenience function to encapsulate a packet in an ethernet frame
 *
 * Creates a PARCBuffer that has an Ethernet header followed by a user-provided byte array.
 * Does not include the frame check sequence.
 *
 * @param [in] length The length of the ccnxPacket to be encapsulated
 * @param [in] ccnxPacket the byte array to put after the ethernet header
 * @param [in] dmac[6] The destination mac
 * @param [in] smac[6] The source mac
 * @param [in] ethertype The ethertype in host byte order
 *
 * @retval non-null An allocated PARCBuffer wrapping an ethernet frame, ready to read
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *mockGenericEther_createFrame(size_t length, const uint8_t ccnxPacket[length], const uint8_t dmac[ETHER_ADDR_LEN], const uint8_t smac[ETHER_ADDR_LEN], uint16_t ethertype);

#endif // Metis_testrig_GenericEther_h
