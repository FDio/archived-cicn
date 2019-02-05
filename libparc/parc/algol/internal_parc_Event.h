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
 * @file parc_EventQueue.h
 * @ingroup events
 * @brief Queue buffer events
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_internal_parc_Event_h
#define libparc_internal_parc_Event_h

#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_EventQueue.h>

/**
 * Map alloc() method call to a PARC internal memory method
 *
 * @param [in] size of memory to allocate
 * @returns NULL on error, memory pointer on success.
 *
 * Example:
 * @code
 * {
 *     return internal_parc_alloc(1024);
 * }
 * @endcode
 *
 */
void *internal_parc_alloc(size_t size);

/**
 * Map realloc() method call to a PARC internal memory method
 *
 * @param [in] pointer to memory to reallocate
 * @param [in] newSize of memory to allocate
 * @returns NULL on error, new memory pointer on success.
 *
 * Example:
 * @code
 * {
 *     return internal_parc_realloc(ptr, 2048);
 * }
 * @endcode
 *
 */
void *internal_parc_realloc(void *pointer, size_t newSize);

/**
 * Map free() method call to a PARC internal memory method
 *
 * @param [in] pointer to memory to free
 *
 * Example:
 * @code
 * {
 *     internal_parc_free(ptr);
 * }
 * @endcode
 *
 */
void internal_parc_free(void *ptr);

/**
 * Verify and initialize libevent
 *
 * Example:
 * @code
 * {
 *     internal_parc_initializeLibevent();
 * }
 * @endcode
 *
 */
void internal_parc_initializeLibevent(void);

/**
 * Convert from/to libevent dispatcher options and PARCEventSchedulerDispatcherType options.
 *
 * Example:
 * @code
 * {
 *     PARCEventSchedulerDispatchType type = internal_eventloop_options_to_PARCEventSchedulerDispatchType(EVLOOP_ONCE);
 *     short evtype = internal_PARCEventSchedulerDispatchType_to_eventloop_options(PARCEventSchedulerDispatchType_LoopOnce);
 * }
 * @endcode
 *
 */
PARCEventSchedulerDispatchType internal_eventloop_options_to_PARCEventSchedulerDispatchType(short evoptions);
short internal_PARCEventSchedulerDispatchType_to_eventloop_options(PARCEventSchedulerDispatchType options);

/**
 * Convert from/to libevent bufferevent options and PARCEventQueueOption.
 *
 * Example:
 * @code
 * {
 *     PARCEventQueueOption parcEventQueueOption = internal_bufferevent_options_to_PARCEventQueueOption(BEV_OPT_CLOSE_ON_FREE);
 *     short buffereventOption = internal_PARCEventQueueOption_to_bufferevent_options(PARCEventQueueOption_CloseOnFree);
 * }
 * @endcode
 *
 */
PARCEventQueueOption internal_bufferevent_options_to_PARCEventQueueOption(short evflags);
short internal_PARCEventQueueOption_to_bufferevent_options(PARCEventQueueOption flags);

/**
 * Convert from/to libevent bufferevent types and PARCEventQueueEventType.
 *
 * Example:
 * @code
 * {
 *     PARCEventQueueEventType parcEventQueueEventType = internal_bufferevent_type_to_PARCEventQueueEventType(BEV_EVENT_READING);
 *     short buffereventOptions = internal_PARCEventQueueEventType_to_bufferevent_type(PARCEventQueueEventType_Reading);
 * }
 * @endcode
 *
 */
PARCEventQueueEventType internal_bufferevent_type_to_PARCEventQueueEventType(short evtypes);
short internal_PARCEventQueueEventType_to_bufferevent_type(PARCEventQueueEventType types);

/**
 * Convert from/to libevent event types and PARCEventType.
 *
 * Example:
 * @code
 * {
 *     PARCEventType parcEventType = internal_libevent_type_to_PARCEventType(EV_READ);
 *     short buffereventOptions = internal_PARCEventType_to_libevent_type(PARCEventType_Read);
 * }
 * @endcode
 *
 */
PARCEventType internal_libevent_type_to_PARCEventType(short evtypes);
short internal_PARCEventType_to_libevent_type(PARCEventType types);

/**
 * Convert from/to libevent priority types and PARCEventPriority.
 *
 * Example:
 * @code
 * {
 *     PARCEventPriority parcEventPriority = internal_libevent_priority_to_PARCEventPriority(0);
 *     short priority = internal_PARCEventPriority_to_libevent_priority(PARCEventPriority_Normal);
 * }
 * @endcode
 *
 */
short internal_PARCEventPriority_to_libevent_priority(PARCEventPriority priority);
PARCEventPriority internal_libevent_priority_to_PARCEventPriority(short evpriority);
#endif // libparc_internal_parc_Event_h
