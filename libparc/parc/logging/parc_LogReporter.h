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
 * @file parc_LogReporter.h
 * @brief An abstract representation of a PARC Log Reporter.
 *
 */
#ifndef PARC_Library_parc_LogReporter_h
#define PARC_Library_parc_LogReporter_h

#include <parc/logging/parc_LogEntry.h>

typedef void (PARCLogReporterAcquire)(void *reporter);

/**
 * A Function that performs the final cleanup and resource deallocation when
 * a PARCLogReporter is no longer needed.
 */
typedef void (PARCLogReporterRelease)(void **reporterP);

/**
 */
typedef void (PARCLogReporterReport)(const PARCLogEntry *reporter);

struct PARCLogReporter;
typedef struct PARCLogReporter PARCLogReporter;

/**
 * Create a new instance of `PARCLogReporter` using the given the functions specified.
 *
 * @param [in] acquire A pointer to a function that performs the Aquire contract.
 * @param [in] release A pointer to a function that performs the Release contract.
 * @param [in] report A pointer to a function that performs the 'report' function.
 * @param [in] privateObject A pointer to a PARCObject that is supplied to the report function when invoked, or NULL.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a valid `PARCLogReporter` instance.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *result = parcLogReporter_Create(&parcLogReporterFile_Acquire,
 *         parcLogReporterFile_Release,
 *         parcLogReporterFile_Report,
 *         parcOutputStream_Acquire(output));
 *     return result;
 * }
 * @endcode
 */
PARCLogReporter *parcLogReporter_Create(PARCLogReporter *(*acquire)(const PARCLogReporter *),
                                        void (*release)(PARCLogReporter **),
                                        void (*report)(PARCLogReporter *, const PARCLogEntry *),
                                        void *privateObject);

/**
 * Increase the number of references to a `PARCLogReporter` instance.
 *
 * Note that new `PARCLogReporter` is not created,
 * only that the given `PARCLogReporter` reference count is incremented.
 * Discard the reference by invoking `parcLogReporter_Release`.
 *
 * @param [in] instance A pointer to a `PARCLogReporter` instance.
 *
 * @return The input `PARCLogReporter` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *reporter = parcLogReporter_Create(&parcLogReporterFile_Acquire,
 *         parcLogReporterFile_Release,
 *         parcLogReporterFile_Report,
 *         parcOutputStream_Acquire(output));
 *
 *     PARCLogReporter *x_2 = parcLogReporter_Acquire(reporter);
 *
 *     parcLogReporter_Release(&reporter);
 *     parcLogReporter_Release(&x_2);
 * }
 * @endcode
 */
PARCLogReporter *parcLogReporter_Acquire(const PARCLogReporter *instance);

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
 * @param [in,out] instancePtr A pointer to a pointer to a `PARCLogReporter`.  The parameter is set to zero.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *reporter = parcLogReporter_Create(&parcLogReporterFile_Acquire,
 *         parcLogReporterFile_Release,
 *         parcLogReporterFile_Report,
 *         parcOutputStream_Acquire(output));
 *
 *     parcLogReporter_Release(&reporter);
 * }
 * @endcode
 */
void parcLogReporter_Release(PARCLogReporter **instancePtr);

/**
 * Report the given PARCLogEntry
 *
 * @param [in] reporter A pointer to a valid PARCLogReporter instance.
 * @param [in] entry A pointer to a valid PARCLogEntry instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcLogReporter_Report(PARCLogReporter *reporter, const PARCLogEntry *entry);

/**
 * Get the private PARCObject supplied when the PARCLogReporter was created.
 *
 * @param [in] reporter A valid PARCLogReporter instance.
 *
 * @return A same pointer supplied when the PARCLogReporter was created.
 *
 */
void *parcLogReporter_GetPrivateObject(const PARCLogReporter *reporter);

#endif
