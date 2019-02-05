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
 * @file parc_LogReporterTextStdout.h
 * @brief A simple log reporter using plain text formatting to standard output.
 *
 */
#ifndef PARC_Library_parc_LogReporterTextStdout_h
#define PARC_Library_parc_LogReporterTextStdout_h

#include <parc/logging/parc_LogReporter.h>

/**
 * Create a new instance of `PARCLogReporter` using standard output.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a valid `PARCLogReporter` instance.
 *
 * Example:
 * @code
 * {
 *     PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
 *
 *     parcLogReporter_Release(&reporter);
 * }
 * <#example#>
 * @endcode
 */
PARCLogReporter *parcLogReporterTextStdout_Create(void);

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
 *     PARCLogReporter *x = parcLogReporterTextStdout_Create();
 *
 *     PARCLogReporter *x_2 = parcLogReporterTextStdout_Acquire(x);
 *
 *     parcLogReporterTextStdout_Release(&x);
 *     parcLogReporterTextStdout_Release(&x_2);
 * }
 * @endcode
 */
PARCLogReporter *parcLogReporterTextStdout_Acquire(const PARCLogReporter *instance);

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
 *     PARCLogReporter *x = parcLogReporterTextStdout_Create();
 *
 *     parcLogReporterTextStdout_Release(&x);
 * }
 * @endcode
 */
void parcLogReporterTextStdout_Release(PARCLogReporter **reporterP);

/**
 * Report the given PARCLogEntry
 *
 * @param [in] reporter A pointer to a valid PARCLogReporter instance.
 * @param [in] entry A pointer to a valid PARCLogEntry instance.
 *
 * @see parcLogReporter_Report
 */
void parcLogReporterTextStdout_Report(PARCLogReporter *reporter, const PARCLogEntry *entry);

#endif
