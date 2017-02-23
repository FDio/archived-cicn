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

/** @file rta_ProtocolStack.h
 *  @brief A set of connectors and components
 *
 * In a Ready To Assemble transport, individual pieces are called connectors
 * and components.  A connector attaches to the API library at the top and
 * to the forwarder at the bottom.  In between the connectors are components.
 *
 * One set of connectors and components is called a protocol stack.
 *
 * A ProtocolStack defines a set of Components linked by bidirectional
 * queues.  A ProtocolStack is defined by the KeyValue set passed to
 * the Transport.  The hash of the KeyValue set selects the protocol stack.
 * If the Transport sees a new hash, it creates a new protocol stack
 * via ProtocolStack_Create().
 *
 * Each API connection calls _Open, which will return a new RtaConnection
 * pointer.  The Transport gives the API an "api_fd", which the Transport
 * translates to the RtaConnection.
 *
 * A protocol stack is implemented as a set of queue pairs between components.
 * There is a fixed sized array called queue_pairs[MAX_STACK_DEPTH].  The
 * queue_pairs[i].pair[RTA_DOWN] end attaches to the upper component.  RTA_DOWN
 * indicates the direction of travel for a write.  queue_pairs[i].pair[RTA_UP]
 * attaches to the lower component.
 *
 * A component only knows its identity (see components.h).  For example, the
 * TLV codec is called CODEC_TLV, and that is the only identity it know.  It does
 * not know the identity of the pieces above or below it.
 *
 * Therefore, when a component calls protocolStack_GetPutQ(stack, CODEC_TLV, RTA_DOWN),
 * it is asking for the queue to write to in the DOWN direction.  This means that
 * we should keep an index by the component name, not by the queue_pairs[] array.
 * Thus, we keep a component_queues[] array that is indexed by the component name.
 *
 * Let's say our stack is API_CONNECTOR, FC_NULL, VERIFY_NULL, CODEC_TLV, FWD_LOCAL.
 * The picture is like this:
 *
 * @code
 *         |
 *         *     <- api_connector managed queue
 *    API_CONNECTOR
 *         *     <- queue_pair[0].pair[DOWN]  <- component_queue[API_CONNECTOR].pair[DOWN]
 *         |
 *         *     <- queue_pair[0].pair[UP]    <- component_queue[FC_NULL].pair[UP]
 *      FC_NULL
 *         *     <- queue_pair[1].pair[DOWN]  <- component_queue[FC_NULL].pair[DOWN]
 *         |
 *         *     <- queue_pair[1].pair[UP]    <- component_queue[VERIFY_NULL].pair[UP]
 *     VERIFY_NULL
 *         *     <- queue_pair[2].pair[DOWN]  <- component_queue[VERIFY_NULL].pair[DOWN]
 *         |
 *         *     <- queue_pair[2].pair[UP]    <- component_queue[CODEC_TLV].pair[UP]
 *     CODEC_TLV
 *         *     <- queue_pair[3].pair[DOWN]  <- component_queue[CODEC_TLV].pair[DOWN]
 *         |
 *         *     <- queue_pair[3].pair[UP]    <- component_queue[FWD_LOCAL].pair[UP]
 *     FWD_LOCAL
 *         *     <- fwd_local managed connection
 *         |
 * @endcode
 *
 * Each component also has a pair of callbacks, one for reading messages flowing down
 * the stack and one for reading messages flowing up the stack.  These are called
 * "downcall_read" for reading messages flowing down and "upcall_read" for messages
 * flowing up.
 *
 * Recall that the direction attributes UP and DOWN in the queues are in terms
 * of WRITES, therefore the directions are opposite for reads.  A component's
 * downcall_read will read from component_queue[X].pair[UP].
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef Libccnx_rta_ProtocolStack_h
#define Libccnx_rta_ProtocolStack_h

#include <parc/algol/parc_ArrayList.h>

#include <parc/algol/parc_EventQueue.h>

#include <ccnx/transport/transport_rta/core/rta_ComponentStats.h>
#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/components.h>
#include <ccnx/transport/transport_rta/core/rta_ComponentQueue.h>
#include <ccnx/transport/transport_rta/commands/rta_Command.h>

struct rta_connection;
struct component_queue;

struct protocol_stack;
typedef struct protocol_stack RtaProtocolStack;

/**
 * Used to assign unique connection id to sockets.  This is just
 * for internal tracking, its not a descriptor.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] stack <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
unsigned rtaProtocolStack_GetNextConnectionId(RtaProtocolStack *stack);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] framework <#description#>
 * @param [in] params <#description#>
 * @param [in] stack_id <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
RtaProtocolStack *rtaProtocolStack_Create(RtaFramework *framework, PARCJSON *params, int stack_id);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] stack <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int rtaProtocolStack_Configure(RtaProtocolStack *stack);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] stack <#description#>
 * @param [in] component <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void *rtaProtocolStack_GetPrivateData(RtaProtocolStack *stack, RtaComponents component);
/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] stack <#description#>
 * @param [in] component <#description#>
 * @param [in] private <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void rtaProtocolStack_SetPrivateData(RtaProtocolStack *stack, RtaComponents component, void *private);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] stack <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
RtaFramework *rtaProtocolStack_GetFramework(RtaProtocolStack *stack);

/**
 * <#One Line Description#>
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
 *
 * @see <#references#>
 */
int rtaProtocolStack_GetStackId(RtaProtocolStack *stack);

/**
 * Opens a connection inside the protocol stack: it calls open() on each component.
 *
 * Returns 0 on success, -1 on error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int rtaProtocolStack_Open(RtaProtocolStack *, struct rta_connection *connection);

/**
 *
 * 0 success, -1 error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int rtaProtocolStack_Close(RtaProtocolStack *, struct rta_connection *conn);

/**
 * <#One Line Description#>
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
 *
 * @see <#references#>
 */
void rtaProtocolStack_Destroy(RtaProtocolStack **stack);

/**
 *  Return the queue used for output for a component in a given direction
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
 *
 * @see <#references#>
 */
PARCEventQueue *rtaProtocolStack_GetPutQueue(RtaProtocolStack *stack,
                                             RtaComponents component,
                                             RtaDirection direction);

/**
 * <#One Line Description#>
 *
 * Domain is the top-level key, e.g. SYSTEM or USER
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
PARCJSON *rtaProtocolStack_GetParam(RtaProtocolStack *stack, const char *domain, const char *key);

/**
 * <#One Line Description#>
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
 *
 * @see <#references#>
 */
RtaComponentStats *rtaProtocolStack_GetStats(const RtaProtocolStack *stack, RtaComponents type);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param stack
 * @param file
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCArrayList *rtaProtocolStack_GetStatistics(const RtaProtocolStack *stack, FILE *file);

/**
 * Look up the symbolic name of the queue.  Do not free the return.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *rtaProtocolStack_GetQueueName(RtaProtocolStack *stack, PARCEventQueue *queue);

/**
 * A state event occured on the given connection, let all the components know.
 *
 * A state changed occured (UP, DOWN, PAUSE, or flow control), notify all the components
 *
 * @param [in] connection The RtaConnection.
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaProtocolStack_ConnectionStateChange(RtaProtocolStack *stack, void *connection);
#endif
