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
 * @file metis_HopByHopFragmenterer.h
 * @brief Implements a fragmenter for a hop-by-hop protocol
 *
 * The hop-by-hop fragmenter can be though of as a bi-directional queue.
 *
 * The receive process takes fragmented packets from the wire and reassembles them.  Once a packet is
 * reassembed, it is put on end of the receive queue.  The user can then pop a message of the top of
 * the receive queue and process it normally.
 *
 * The send process takes a potentially large input packet and fragments it in to pieces that are places
 * in order on the send queue.  The caller can then pop messages of the head of the send queue and
 * put them on the wire.
 *
 * In the next update, we probalby want to make the send queue and receive queue external so I/O and
 * message processing threads can access them directly.
 *
 */

#ifndef __Metis__metis_HopByHopFragmenter__
#define __Metis__metis_HopByHopFragmenter__

#include <stdbool.h>
#include <ccnx/forwarder/metis/core/metis_Logger.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>

struct metis_hopbyhop_fragment;
typedef struct metis_hopbyhop_fragment MetisHopByHopFragmenter;

/**
 * Creates a fragmentation class for a specific session
 *
 * The fragmenter is specific to a given flow (i.e. source-destination-ethertype tuple).
 * It is the responsibility of the caller to create the appropriate number of fragmenters
 * and classify packets in to the right fragmenter.
 *
 * @param [in] logger The logger to use for output
 * @param [in] mtu The MTU to use for send operations
 *
 * @return non-null An allocted MetisHopByHopFragmenter
 * @return null An error
 *
 * Example:
 * @code
 * {
 *    MetisHopByHopFragmenter *fragmenter = metisHopByHopFragmenter_Create(logger, mtu);
 *    metisHopByHopFragmenter_Release(&fragmenter);
 * }
 * @endcode
 */
MetisHopByHopFragmenter *metisHopByHopFragmenter_Create(MetisLogger *logger, unsigned mtu);

/**
 * Release a reference to the fragmenter
 *
 * Will destroy any packets in the receive and send queues.
 *
 * @param [in,out] fragmenterPtr A pointer to an allocated MetisHopByHopFragmenter
 *
 * Example:
 * @code
 * {
 *    MetisHopByHopFragmenter *fragmenter = metisHopByHopFragmenter_Create(logger, mtu);
 *    metisHopByHopFragmenter_Release(&fragmenter);
 * }
 * @endcode
 */
void metisHopByHopFragmenter_Release(MetisHopByHopFragmenter **fragmenterPtr);

/**
 * Receives a message as part of the fragmentation session
 *
 * Receives a fragment.  If this causes a reassembly to complete, the completed packet
 * will be placed in the receive queue and may be accessed by metisHopByHopFragmenter_PopReceiveQueue().
 * The caller is reponsible for releasing message.
 *
 * If a non-fragment packet is received, it is placed directly on the receive queue.
 *
 * The caller is responsible for releasing the message.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 * @param [in] message An allocated MetisMessage
 *
 * @return true The receive buffer has an assembled packet ready for read
 * @return false the receive buffer does not have a complete packet ready.
 *
 * Example:
 * @code
 * {
 *   void
 *   acceptFragment(MetisHopByHopFragmenter *fragmenter, MetisMessage *message)
 *   {
 *      bool receiveQueueNotEmpty = metisHopByHopFragmenter_Receive(fragmenter, message);
 *      if (receiveQueueNotEmpty) {
 *         MetisMessage *assembled = NULL;
 *         while ((assembled = metisHopByHopFragmenter_PopReceiveQueue(fragmenter)) != NULL) {
 *            etherListener->stats.framesReassembled++;
 *            metisForwarder_Receive(etherListener->metis, assembled);
 *         }
 *      }
 *   }
 * }
 * @endcode
 */
bool metisHopByHopFragmenter_Receive(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message);

/**
 * Pops the top assembed message from the receive queue
 *
 * Reads the top reassembled packet from the receive queue.  The caller must
 * release the returned message.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 *
 * @return NULL The receive queue is empty (i.e. the current reassembly is not complete)
 * @return non-null A re-assembed message
 *
 * Example:
 * @code
 * {
 *   void
 *   acceptFragment(MetisHopByHopFragmenter *fragmenter, MetisMessage *message)
 *   {
 *      bool receiveQueueNotEmpty = metisHopByHopFragmenter_Receive(fragmenter, message);
 *      if (receiveQueueNotEmpty) {
 *         MetisMessage *assembled = NULL;
 *         while ((assembled = metisHopByHopFragmenter_PopReceiveQueue(fragmenter)) != NULL) {
 *            etherListener->stats.framesReassembled++;
 *            metisForwarder_Receive(etherListener->metis, assembled);
 *         }
 *      }
 *   }
 * }
 * @endcode
 */
MetisMessage *metisHopByHopFragmenter_PopReceiveQueue(MetisHopByHopFragmenter *fragmenter);

/**
 * Adds a message to the send buffer
 *
 * This may make multiple references to the original message where each fragment is
 * pointing as an extent in to the original message.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 * @param [in] message An allocated MetisMessage
 *
 * @return true The message was fragmented and put on the send queue
 * @return false An error
 *
 * Example:
 * @code
 * {
 *   void sendFragments(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message) {
 *      bool success = metisHopByHopFragmenter_Send(fragmenter, message);
 *
 *      MetisMessage *fragment;
 *      while (success && (fragment = metisHopByHopFragmenter_PopSendQueue(fragmenter)) != NULL) {
 *         success = _sendFrame(fragment);
 *         metisMessage_Release(&fragment);
 *      }
 *
 *      // if we failed, drain the other fragments
 *      if (!success) {
 *         while ((fragment = metisHopByHopFragmenter_PopSendQueue(fragmenter)) != NULL) {
 *         metisMessage_Release(&fragment);
 *      }
 *   }
 *   // caller must release message
 * }
 * @endcode
 */
bool metisHopByHopFragmenter_Send(MetisHopByHopFragmenter *fragmenter, MetisMessage *message);

/**
 * Pops the next message to send to the wire from the send queue
 *
 * Returns the front of the Send FIFO queue of fragments that should be
 * sent on the wire.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 *
 * @return null there is no message awaiting transmit
 * @return non-null A message to send
 *
 * Example:
 * @code
 * {
 *   void sendIdleFragment(MetisHopByHopFragmenter *fragmenter) {
 *      bool success = metisHopByHopFragmenter_SendIdle(fragmenter);
 *
 *      MetisMessage *fragment;
 *      while (success && (fragment = metisHopByHopFragmenter_PopSendQueue(fragmenter)) != NULL) {
 *         success = _sendFrame(fragment);
 *         metisMessage_Release(&fragment);
 *      }
 *   }
 * }
 * @endcode
 */
MetisMessage *metisHopByHopFragmenter_PopSendQueue(MetisHopByHopFragmenter *fragmenter);
#endif /* defined(__Metis__metis_HopByHopFragmenter__) */
