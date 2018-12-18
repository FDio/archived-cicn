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
 * @file parc_LogReporterFile.h
 * @brief <#Brief Description#>
 *
 */
#ifndef __PARC_Library__parc_LogReporterFile__
#define __PARC_Library__parc_LogReporterFile__

#include <parc/logging/parc_LogReporter.h>
#include <parc/algol/parc_OutputStream.h>

/**
 * Create a new instance of `PARCLogReporter` using the given {@link PARCOutputStream}.
 *
 * @param [in] output A pointer to a valid `PARCOutputStream` instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a valid `PARCLogReporter` instance.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
 *     PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(out);
 *     parcOutputStream_Release(&out);
 *
 *     parcLogReporter_Release(&reporter);
 * }
 * <#example#>
 * @endcode
 */
PARCLogReporter *parcLogReporterFile_Create(PARCOutputStream *output);

/**
 * Increase the number of references to a `PARCLogReporter` instance.
 *
 * Note that new `PARCLogReporter` is not created,
 * only that the given `PARCLogReporter` reference count is incremented.
 * Discard the reference by invoking `parcLogEntry_Release`.
 *
 * @param [in] instance A pointer to a `PARCLogReporter` instance.
 *
 * @return The input `PARCLogReporter` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *x = parcLogReporterFile_Create(...);
 *
 *     PARCLogReporter *x_2 = parcLogReporterFile_Acquire(x);
 *
 *     parcLogReporterFile_Release(&x);
 *     parcLogReporterFile_Release(&x_2);
 * }
 * @endcode
 */
PARCLogReporter *parcLogReporterFile_Acquire(const PARCLogReporter *instance);

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
 * @param [in,out] reporterP A pointer to a PARCLogReporter instance pointer, which will be set to zero on return.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *x = parcLogReporterFile_Create(...);
 *
 *     parcLogReporterFile_Release(&x);
 * }
 * @endcode
 */
void parcLogReporterFile_Release(PARCLogReporter **reporterP);

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
void parcLogReporterFile_Report(PARCLogReporter *reporter, const PARCLogEntry *entry);
#endif /* defined(__PARC_Library__parc_LogReporterFile__) */
