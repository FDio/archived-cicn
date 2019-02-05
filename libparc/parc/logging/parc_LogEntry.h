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
 * @file parc_LogEntry.h
 * @brief Basic Log Entry implementation
 *
 * PARCLogEntry instances contain logging information in a single message.
 *
 * Each instance contains:
 * * A log level (see PARCLogLevel).
 * * An integer version number denoting the version of the syslog protocol specification (1).
 * * A timestamp representable as an RFC 3339 Timestamp.
 * * A hostname identifing the machine that originally sent the message.
 * * An application name identifing the device or application that originated the message.
 * * A process identifier having specific meaning,
 *    except that a change in the value indicates there has been a discontinuity in a series of
 *    otherwise linear PARCLogEntry instances.
 * * A message identifier as a string without further semantics other than identifing the type of message.
 *
 */
#ifndef PARC_Library_parc_LogEntry_h
#define PARC_Library_parc_LogEntry_h

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <parc/algol/parc_Buffer.h>


struct PARCLogEntry;
typedef struct PARCLogEntry PARCLogEntry;

#include <parc/logging/parc_LogLevel.h>

/**
 * Create a PARCLogEntry instance.
 *
 * @param [in] level A log level (see PARCLogLevel).
 * * An integer version number denoting the version of the syslog protocol specification (1).
 * @param [in] timeStamp The timestamp for the PARCLogEntry.
 * @param [in] hostName The hostname identifing the machine that originally sent the message.
 * @param [in] applicationName The application name identifing the device or application that originated the message.
 * @param [in] processId An identifier having no specific meaning,
 *    except that a change in the value indicates there has been a discontinuity in a series of
 *    otherwise linear PARCLogEntry instances.
 * @param [in] messageId A message identifier for the type of message.
 * @param [in] payload The message component of the LogEntry.
 *
 * @return non-NULL A valid instance of PARCLogEntry.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCLogEntry *parcLogEntry_Create(PARCLogLevel level,
                                  const char *hostName,
                                  const char *applicationName,
                                  const char *processId,
                                  const uint64_t messageId,
                                  const struct timeval timeStamp,
                                  PARCBuffer *payload);

/**
 * Increase the number of references to a `PARCLogEntry` instance.
 *
 * Note that new `PARCLogEntry` is not created,
 * only that the given `PARCLogEntry` reference count is incremented.
 * Discard the reference by invoking `parcLogEntry_Release`.
 *
 * @param [in] instance A pointer to a `PARCLogEntry` instance.
 *
 * @return The input `PARCLogEntry` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCLogEntry *x = parcLogEntry_Create(...);
 *
 *     PARCLogEntry *x_2 = parcLogEntry_Acquire(x);
 *
 *     parcLogEntry_Release(&x);
 *     parcLogEntry_Release(&x_2);
 * }
 * @endcode
 */
PARCLogEntry *parcLogEntry_Acquire(const PARCLogEntry *instance);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] entryPtr A pointer to a pointer to a PARCLogEntry.  The parameter is set to zero.
 *
 * Example:
 * @code
 * {
 *     PARCLogEntry *entry = parcLogEntry_Create(...)
 *
 *     parcLogEntry_Release(&entry);
 * }
 * @endcode
 */
void parcLogEntry_Release(PARCLogEntry **entryPtr);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] entry A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCLogEntry *entry = parcLogEntry_Create(...)
 *
 *     char *string = parcLogEntry_ToString(entry);
 *     printf("%s\n", string);
 *     parcMemory_Deallocate(&string);
 *
 *     parcLogEntry_Release(&entry);
 * }
 * @endcode
 *
 */
char *parcLogEntry_ToString(const PARCLogEntry *entry);

/**
 * Get the payload of the specified PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return A pointer to the payload of the PARCLogEntry.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *parcLogEntry_GetPayload(const PARCLogEntry *instance);

/**
 * Get the timestamp of the specified PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return A pointer to the struct timeval of the PARCLogEntry.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const struct timeval *parcLogEntry_GetTimeStamp(const PARCLogEntry *instance);

/**
 * Get the PARCLogLevel of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The PARCLogLevel of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCLogLevel parcLogEntry_GetLevel(const PARCLogEntry *instance);

/**
 * Get the PARCLogLevel of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The version number of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
int parcLogEntry_GetVersion(const PARCLogEntry *instance);

/**
 * Get the host-name of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The application name of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcLogEntry_GetHostName(const PARCLogEntry *instance);

/**
 * Get the application-name of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The application name of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcLogEntry_GetApplicationName(const PARCLogEntry *instance);

/**
 * Get the process-id of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The process-id of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
const char *parcLogEntry_GetProcessName(const PARCLogEntry *instance);

/**
 * Get the message-id of the given PARCLogEntry.
 *
 * @param [in] instance A pointer to a valid instance of PARCLogEntry.
 *
 * @return The message-id of the given PARCLogEntry.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
uint64_t parcLogEntry_GetMessageId(const PARCLogEntry *instance);

#endif
