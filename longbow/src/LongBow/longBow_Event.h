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
 * @file longBow_Event.h
 * @ingroup internals
 * @brief LongBow Event Support.
 *
 * LongBow assertions, traps and tests induce "events" which are experienced by the programme runtime as signals or long-jumps.
 *
 */
#ifndef LongBow_longBow_Event_h
#define LongBow_longBow_Event_h

#include <stdlib.h>
#include <stdbool.h>

#include <LongBow/longBow_EventType.h>
#include <LongBow/longBow_Location.h>
#include <LongBow/longBow_Backtrace.h>

struct longbow_event;

/**
 * @typedef LongBowEvent
 */
typedef struct longbow_event LongBowEvent;

/**
 * Get the `LongBowEventType` of a LongBowEvent.
 *
 * @param [in] event A `LongBowEvent` instance.
 *
 * @return A pointer to the LongBowEventType of the event.
 *
 * Example:
 * @code
 * {
 *     LongBowEvent *event = ...
 *     LongBowEventType type = longBowEvent_GetEventType(event);
 * }
 * @endcode
 */
const LongBowEventType *longBowEvent_GetEventType(const LongBowEvent *event);

/**
 * Create a `LongBowEvent`.
 *
 * Creating a `LongBowEvent` records runtime data and used by report facilities.
 * This does not actually cause the process to assert the assertion.
 *
 * @param [in] eventType The LongBowEventType for this event.
 * @param [in] location A LongBowLocation instance recording the location of the event.
 * @param [in] kind A string representing the kind of event.
 * @param [in] message A message to display. This will be freed via free(3).
 * @param [in] backtrace A pointer to a valid LongBowBacktrace instance.
 *
 * @return An allocated LongBowEvent which must be destroyed via `longBowEvent_Destroy()`.
 */
LongBowEvent *longBowEvent_Create(const LongBowEventType *eventType, const LongBowLocation *location, const char *kind, const char *message, const LongBowBacktrace *backtrace);

/**
 * Destroy a `LongBowEvent`.
 *
 * The value pointed to by `eventPtr`, is set to `NULL.
 *
 * @param [in,out] eventPtr The `LongBowEvent` instance to destroy and NULLify.
 */
void longBowEvent_Destroy(LongBowEvent **eventPtr);

/**
 * Get the `LongBowLocation` associated with this `LongBowEvent` instance.
 *
 * @param [in] event A `LongBowEvent` instance.
 *
 * @return A pointer to the `LongBowLocation` instance for the given `LongBowEvent`.
 */
const LongBowLocation *longBowEvent_GetLocation(const LongBowEvent *event);

/**
 * Get the name.
 *
 * @param [in] event A `LongBowEvent` instance.
 *
 * @return The name of the given LongBowEvent.
 */
const char *longBowEvent_GetName(const LongBowEvent *event);

/**
 * Get a pointer to the string representing the kind of this event.
 *
 * Currently the kind is only a static string set when creating a `LongBowEvent`.
 *
 * @param [in] event A pointer to a `LongBowEvent` instance.
 *
 * @return non-NULL The pointer to the string representing the kind of this event.
 * @return NULL The kind was not set.
 *
 * @see longBowEvent_Create
 */
const char *longBowEvent_GetKind(const LongBowEvent *event);

/**
 * Retrieve the message associated with this `LongBowEvent` instance.
 *
 * @param [in] event A `LongBowEvent` instance.
 *
 * @return The message associated with the given `LongBowEvent`.
 */
const char *longBowEvent_GetMessage(const LongBowEvent *event);

/**
 * Get the `LongBowBacktrace` instance for the given `LongBowEvent` instance.
 *
 * @param [in] event A pointer to a valid LongBowEvent instance.
 *
 * @return A pointer to a LongBowBacktrace instance.
 */
const LongBowBacktrace *longBowEvent_GetBacktrace(const LongBowEvent *event);

/**
 * Get an array of nul-terminated C strings containing the symbolic representation of the given `LongBowEvent` stack backtrace.
 * The length of the array is provided by `longBowEvent_GetCallStackLength`
 *
 * @param [in] event A pointer to a valid `LongBowEvent` instance.
 *
 * @return non-NULL An array of nul-terminated C strings
 * @see longBowEvent_GetCallStackLength
 */
char **longBowEvent_CreateSymbolicCallstack(const LongBowEvent *event);

/**
 * Retrieve the call stack length associated with this `LongBowEvent` instance.
 *
 * @param [in] event A `LongBowEvent` instance.
 *
 * @return The length of the call stack.
 */
size_t longBowEvent_GetCallStackLength(const LongBowEvent *event);
#endif // LongBow_longBow_Event_h
