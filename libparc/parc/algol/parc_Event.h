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
 * @file parc_Event.h
 * @ingroup events
 * @brief Event management
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_Event_h
#define libparc_parc_Event_h

/**
 * Current implementation based on top of libevent2
 */

#include <parc/algol/parc_EventScheduler.h>

/**
 * @typedef PARCEventType
 * @brief An enumeration of event types, with an additional persist flag
 */
typedef enum {
    PARCEventType_None = 0x00,
    PARCEventType_Timeout = 0x01,
    PARCEventType_Read = 0x02,
    PARCEventType_Write = 0x04,
    PARCEventType_Signal = 0x08,
    PARCEventType_Persist = 0x10,
    PARCEventType_EdgeTriggered = 0x20
} PARCEventType;

/**
 * @typedef PARCEventPriority
 * @brief Priority flags for queue scheduling, these currently match the RTA_*_PRIORITY
 * this will eventually be replaced.
 */
typedef enum {
    PARCEventPriority_Maximum = 0,
    PARCEventPriority_Normal = 1,
    PARCEventPriority_Minimum = 2,
    PARCEventPriority_NumberOfPriorities = 3
} PARCEventPriority;

/**
 * @typedef PARCEvent_Callback
 * @brief Event callback definition
 */
typedef void (PARCEvent_Callback)(int fileDescriptor, PARCEventType type, void *user_data);

/**
 * @typedef PARCEvent
 * @brief A structure containing private libevent state data variables
 */
typedef struct PARCEvent PARCEvent;

/**
 * Create a new PARCEvent instance.
 *
 * A new PARCEvent instance is returned.
 *
 * @param [in] parcEventScheduler base instance
 * @param [in] fileDescriptor file descriptor to monitor
 * @param [in] events to catch
 * @param [in] callback function
 * @param [in] callbackArgs function private arguments
 * @returns A pointer to the a PARCEvent instance.
 *
 * Example:
 * @code
 * static void
 * _read_callback(int fileDescriptor, PARCEventType type, void *args)
 * {
 * }
 *
 * {
 *     PARCEventScheduler *eventScheduler = parcEventScheduer_Create();
 *     PARCEvent *event = parcEvent_Create(eventScheduler, fileDescriptor, PARCEvent_ReadEvent, _read_callback, _read_callback_args);
 * }
 * @endcode
 *
 */
PARCEvent *parcEvent_Create(PARCEventScheduler *parcEventScheduler, int fileDescriptor, PARCEventType events, PARCEvent_Callback *callback, void *callbackArgs);

/**
 * Start an event instance.
 *
 * @param [in] parcEvent instance to start
 * @returns -1 on error, 0 on success if nothing changed in the parcEvent backend, and 1 on success if something did.
 *
 * Example:
 * @code
 * startEvent(PARCEvent *parcEvent)
 * {
 *     return parcEvent_Start(parcEvent);
 * }
 * @endcode
 *
 */
int parcEvent_Start(PARCEvent *parcEvent);

/**
 * Stop a parcEvent instance.
 *
 * @param [in] parcEvent instance to stop
 * @returns -1 on error, 0 on success.
 *
 * Example:
 * @code
 * removeEvent(PARCEvent *parcEvent)
 * {
 *     return parcEvent_Stop(parcEvent);
 * }
 * @endcode
 *
 */
int parcEvent_Stop(PARCEvent *parcEvent);

/**
 * Poll if an event is available to process
 *
 * @param [in] parcEvent instance to stop
 * @param [in] event type to poll for
 * @returns -1 on error, 0 on success.
 *
 * Example:
 * @code
 * pollReadEvent(PARCEvent *parcEvent)
 * {
 *     return parcEvent_Poll(parcEvent, PARCEvent_ReadEvent);
 * }
 * @endcode
 *
 */
int parcEvent_Poll(PARCEvent *parcEvent, PARCEventType event);

/**
 * Destroy a parcEvent instance.
 *
 * @param [in] parcEvent address of instance to destroy.
 *
 * Example:
 * @code
 * {
 *     parcEvent_Destroy(&parcEvent);
 * }
 * @endcode
 *
 */
void parcEvent_Destroy(PARCEvent **parcEvent);

/**
 * Set a parcEvent instances priority.
 *
 * @param [in] parcEvent instance to modify
 * @param [in] priority to set to
 * @returns -1 on error, 0 on success.
 *
 * Example:
 * @code
 * {
 *     return parcEvent_SetPriority(parcEvent, priority);
 * }
 * @endcode
 *
 */
int parcEvent_SetPriority(PARCEvent *parcEvent, PARCEventPriority priority);

/**
 * Turn on debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEvent_EnableDebug();
 * }
 * @endcode
 *
 */
void parcEvent_EnableDebug(void);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEvent_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEvent_DisableDebug(void);
#endif // libparc_parc_Event_h
