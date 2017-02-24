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
 * @file rta_Logger.h
 * @brief Logger for the Rta transport
 *
 * A facility based logger to allow selective logging from different parts of Rta
 *
 */

#ifndef Rta_rta_Logger_h
#define Rta_rta_Logger_h

#include <sys/time.h>
#include <stdarg.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/logging/parc_LogLevel.h>
#include <parc/logging/parc_LogReporter.h>
#include <parc/algol/parc_Clock.h>

struct rta_logger;
typedef struct rta_logger RtaLogger;

/**
 * Framework - Overall framework
 * ApiConnector - API Connector
 * Flowcontrol - Flow controller
 * Codec - Codec and verification/signing
 * ForwarderConnector - Forwarder connector
 */
typedef enum {
    RtaLoggerFacility_Framework,
    RtaLoggerFacility_ApiConnector,
    RtaLoggerFacility_Flowcontrol,
    RtaLoggerFacility_Codec,
    RtaLoggerFacility_ForwarderConnector,
    RtaLoggerFacility_END          // sentinel value
} RtaLoggerFacility;

/**
 * Returns a string representation of a facility
 *
 * Do not free the returned value.
 *
 * @param [in] facility The facility to change to a string
 *
 * @retval string A string representation of the facility
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *rtaLogger_FacilityString(RtaLoggerFacility facility);

/**
 * Returns a string representation of a log level
 *
 * Do not free the returned value.
 *
 * @param [in] level The level to change to a string
 *
 * @retval string A string representation of the level
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *rtaLogger_LevelString(PARCLogLevel level);

/**
 * Create a logger that uses a given writer and clock
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] writer The output writer
 * @param [in] clock The clock to use for log messages
 *
 * @retval non-null An allocated logger
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaLogger *rtaLogger_Create(PARCLogReporter *reporter, const PARCClock *clock);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaLogger_Release(RtaLogger **loggerPtr);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaLogger *rtaLogger_Acquire(const RtaLogger *logger);

/**
 * Sets the minimum log level for a facility
 *
 * The default log level is ERROR.  For a message to be logged, it must be of equal
 * or higher log level.
 *
 * @param [in] logger An allocated logger
 * @param [in] facility The facility to set the log level for
 * @param [in] The minimum level to log
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
 *    RtaLogger *logger = rtaLogger_Create(reporter, parcClock_Wallclock());
 *    parcLogReporter_Release(&reporter);
 *    rtaLogger_SetLogLevel(logger, RtaLoggerFacility_IO, PARCLogLevel_Warning);
 * }
 * @endcode
 */
void rtaLogger_SetLogLevel(RtaLogger *logger, RtaLoggerFacility facility, PARCLogLevel minimumLevel);

/**
 * Tests if the log level would be logged
 *
 * If the facility would log the given level, returns true.  May be used as a
 * guard around expensive logging functions.
 *
 * @param [in] logger An allocated logger
 * @param [in] facility The facility to test
 * @param [in] The level to test
 *
 * @retval true The given facility would log the given level
 * @retval false A message of the given level would not be logged
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool rtaLogger_IsLoggable(const RtaLogger *logger, RtaLoggerFacility facility, PARCLogLevel level);

/**
 * Log a message
 *
 * The message will only be logged if it is loggable (rtaLogger_IsLoggable returns true).
 *
 * @param [in] logger An allocated RtaLogger
 * @param [in] facility The facility to log under
 * @param [in] level The log level of the message
 * @param [in] module The specific module logging the message
 * @param [in] format The message with varargs
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaLogger_Log(RtaLogger *logger, RtaLoggerFacility facility, PARCLogLevel level, const char *module, const char *format, ...);

/**
 * Switch the logger to a new reporter
 *
 * Will close the old reporter and re-setup the internal loggers to use the new reporter.
 * All current log level settings are preserved.
 *
 * @param [in] logger An allocated RtaLogger
 * @param [in] reporter An allocated PARCLogReporter
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaLogger_SetReporter(RtaLogger *logger, PARCLogReporter *reporter);

/**
 * Set a new clock to use with the logger
 *
 * The logger will start getting the time (logged as the messageid) from the specified clock
 *
 * @param [in] logger An allocated RtaLogger
 * @param [in] clock An allocated PARCClock
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaLogger_SetClock(RtaLogger *logger, PARCClock *clock);
#endif // Rta_rta_Logger_h
