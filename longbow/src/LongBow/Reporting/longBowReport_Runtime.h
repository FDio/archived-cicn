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
 * @file Reporting/longBowReport_Runtime.h
 * @ingroup reporting
 * @brief The LongBow Runtime Report Generator.
 *
 * This header specifies the interface for an implementation of a LongBow Test Report generator.
 * Different implementations of a Test Report generator are used to connect to external environments to hook in
 * LongBow unit tests within a larger framework like an IDE or continuous integration system.
 *
 * There may be many different ways to report the summary of a LongBow Unit Test,
 * and the different ways implement the functions prescribed here.
 * The resulting object files are then linked with the unit-test according to the kind of report needed.
 *
 */
#ifndef LONGBOW_REPORT_RUNTIME_H_
#define LONGBOW_REPORT_RUNTIME_H_

#include <LongBow/longBow_Event.h>

#include <LongBow/runtime.h>
#include <LongBow/unit-test.h>

struct longbow_report_config;
/**
 * @brief Configuration for a LongBow Test.
 */
typedef struct longbow_report_config LongBowReportConfig;

/**
 * @struct longbow_report_config
 * @brief The configuration information for a LongBow Test Report.
 */
struct longbow_report_config {
    struct {
        unsigned int untested : 1;
        unsigned int succeeded : 1;
        unsigned int warned : 1;
        unsigned int teardown_warned : 1;
        unsigned int skipped : 1;
        unsigned int unimplemented : 1;
        unsigned int failed : 1;
        unsigned int stopped : 1;
        unsigned int teardown_failed : 1;
        unsigned int setup_failed : 1;
        unsigned int signalled : 1;
    } suppress_report;  /**< Bit fields representing which report to suppress. */
};

/**
 * Create a LongBowReportConfiguration from a set of parameters.
 *
 * @param [in] argc The number of parameters in the argv array.
 * @param [in] argv An array of C strings.
 *
 * @return An allocated LongBowReportConfiguration instance that must be dellocted via longBowReport_Destroy.
 *
 * Example:
 * @code
 * {
 *     char *argv[2] = { "arg1", "arg2" };
 *     LongBowReportConfig *report = longBowReport_Create(2, argv);
 * }
 * @endcode
 */
LongBowReportConfig *longBowReportRuntime_Create(int argc, char *argv[]);

/**
 * Destroy a LongBowReportConfig instance.
 *
 * @param [in,out] configPtr A pointer to a LongBowReportConfig pointer. The value of configPtr will be set to zero.
 *
 * Example:
 * @code
 * {
 *     char *argv[2] = { "arg1", "arg2" };
 *     LongBowReportConfig *report = longBowReport_Create(2, argv);
 *     LongBowReport_Destroy(&report);
 * }
 * @endcode
 */
void longBowReportRuntime_Destroy(LongBowReportConfig **configPtr);

/**
 * Report a LongBowEvent.
 *
 * @param [in] event A pointer to a valid LongBowEvent instance.
 */
void longBowReportRuntime_Event(const LongBowEvent *event);

/**
 * Report a message.
 *
 * @param [in] message A pointer to a nul-terminated C string.
 */
void longBowReportRuntime_Message(const char *message, ...);

/**
 * Report an error message.
 *
 * An error message reports an unrecoverable error.
 *
 * @param [in] message A pointer to a nul-terminated C string.
 */
void longBowReportRuntime_Error(const char *message, ...);

/**
 * Report an error message.
 *
 * An error message reports an recoverable warning.
 *
 * @param [in] message A pointer to a nul-terminated C string.
 */
void longBowReportRuntime_Warning(const char *message, ...);

/**
 * Format a struct timeval structure.
 *
 * @param [in] time A struct timeval value.
 *
 * @return An allocated nul-terminated C string that must be freed via stdlib free(3).
 */
char *longBowReportRuntime_TimevalToString(const struct timeval time);

/**
 * Format a struct rusage struture.
 *
 * @param [in] rusage A pointer to a valid `struct rusage` instance.
 * @return An allocated nul-terminated C string that must be freed via stdlib free(3).
 */
char *longBowReportRuntime_RUsageToString(const struct rusage *rusage);
#endif // LONGBOW_REPORT_RUNTIME_H_
