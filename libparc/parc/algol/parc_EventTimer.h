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
 * @file parc_EventTimer.h
 * @ingroup events
 * @brief Timer events
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_EventTimer_h
#define libparc_parc_EventTimer_h

#include <parc/algol/parc_Event.h>

typedef void (PARCEventTimer_Callback)(int fd, PARCEventType type, void *user_data);

typedef struct PARCEventTimer PARCEventTimer;

/**
 * Create a new timer event instance.
 *
 * The new event instance is returned.
 *
 * @param [in] parcEventScheduler - scheduler instance to attach to
 * @param [in] flags - event flags
 * @param [in] callback - timer callback function
 * @param [in] callbackArguments - private arguments passed to callback
 * @returns A pointer to the a PARCEventTimer instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventTimer *event = parcEvent_Create(parcEventScheduler, PARCEvent_None, callback, callbackArguments);
 * }
 * @endcode
 *
 */
PARCEventTimer *parcEventTimer_Create(PARCEventScheduler *parcEventScheduler,
                                      PARCEventType flags,
                                      PARCEventTimer_Callback *callback,
                                      void *callbackArguments);

/**
 * Position a timer event instance to be scheduled.
 *
 * @param [in] parcEventTimer - The newly created event instance and a timeout value.
 * @param [in] timeout - time to wait for event, or NULL to wait forever.
 * @returns -1 on error, 0 on success if nothing changed in the event backend, and 1 on success if something did.
 *
 * Example:
 * @code
 * addEvent(PARCEventTimer *parcEventTimer)
 * {
 *     struct timeval timeout = {5, 0};
 *     int result = parcEventTimer_Start(parcEventTimer, &timeout);
 * }
 * @endcode
 *
 */
int parcEventTimer_Start(PARCEventTimer *parcEventTimer, struct timeval *timeout);

/**
 * Stop a timer event instance.
 *
 * @param [in] parcEventTimer - The newly created event instance and a timeout value.
 * @returns 0 on success
 *
 * Example:
 * @code
 * removeEvent(PARCEventTimer *parcEventTimer)
 * {
 *     parcEventTimer_Stop(parcEventTimer);
 * }
 * @endcode
 *
 */
int parcEventTimer_Stop(PARCEventTimer *parcEventTimer);

/**
 * Destroy an event instance.
 *
 * The event instance is passed in.
 *
 * @param [in] parcEventTimer the instance to destroy.
 *
 * Example:
 * @code
 * {
 *     parcEventTimer_Destroy(&parcEventTimer);
 * }
 * @endcode
 *
 */
void parcEventTimer_Destroy(PARCEventTimer **parcEventTimer);

/**
 * Turn on debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventTimer_EnableDebug();
 * }
 * @endcode
 *
 */
void parcEventTimer_EnableDebug(void);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventTimer_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEventTimer_DisableDebug(void);
#endif // libparc_parc_EventTimer_h
