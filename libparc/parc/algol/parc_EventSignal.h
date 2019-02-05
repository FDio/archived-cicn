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
 * @file parc_EventSignal.h
 * @ingroup events
 * @brief Signal events
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_EventSignal
#define libparc_parc_EventSignal

#include <parc/algol/parc_Event.h>

typedef void (PARCEventSignal_Callback)(int fd, PARCEventType type, void *user_data);

typedef struct PARCEventSignal PARCEventSignal;

/**
 * Create a new signal event instance.
 *
 * The new event instance is returned.
 *
 * @param [in] parcEventScheduler - scheduler instance
 * @param [in] signal - signal to catch
 * @param [in] flags - event flags
 * @param [in] callback - event callback
 * @param [in] callbackArguments - private arguments passed to callback
 * @returns A pointer to the a PARCEventSignal instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, 0, callback, callbackArguments);
 * }
 * @endcode
 *
 */
PARCEventSignal *parcEventSignal_Create(PARCEventScheduler *parcEventScheduler,
                                        int signal, PARCEventType flags,
                                        PARCEventSignal_Callback *callback,
                                        void *callbackArguments);

/**
 * Prepare a parcEventSignal instance to be scheduled.
 *
 * @returns -1 on error, 0 on success if nothing changed in the event backend, and 1 on success if something did.
 * @param [in] parcEventSignal the newly created event instance.
 *
 * Example:
 * @code
 * addEvent(PARCEventSignal *parcEventSignal)
 * {
 *     int result = parcEventSignal_Start(parcEventSignal);
 * }
 * @endcode
 *
 */
int parcEventSignal_Start(PARCEventSignal *parcEventSignal);

/**
 * Stop a parcEventSignal instance.
 *
 * @param [in] parcEventSignal - The newly created event instance.
 *
 * Example:
 * @code
 * removeEvent(PARCEventSignal *parcEventSignal)
 * {
 *     parcEventSignal_Stop(parcEventSignal);
 * }
 * @endcode
 *
 */
int parcEventSignal_Stop(PARCEventSignal *parcEventSignal);

/**
 * Destroy a parcEventSignal instance.
 *
 * The event instance is passed in.
 *
 * @param [in] parcEventSignal The instance to destroy.
 *
 * Example:
 * @code
 * {
 *     parcEventSignal_Destroy(&eparcEventSignal vent);
 * }
 * @endcode
 *
 */
void parcEventSignal_Destroy(PARCEventSignal **parcEventSignal);

/**
 * Turn on debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventSignal_EnableDebug();
 * }
 * @endcode
 *
 */
void parcEventSignal_EnableDebug(void);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventSignal_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEventSignal_DisableDebug(void);
#endif // libparc_parc_EventSignal_h
