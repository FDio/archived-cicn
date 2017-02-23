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
 * @file parc_Log.h
 * @brief Event logging.
 *
 * This is an logging mechanism patterned after the Syslog logging protocol (RFC 5424),
 * and influenced by `java.util.logging` and Apache Log4J.
 *
 * The lifecycle of a `PARCLog` starts with creating an instance via `parcLog_Create`
 * and calling the various functions to emit log messages.
 *
 * Finally the log is released via `parcLog_Release` which ensures
 * that any queued log messages are transmitted and resources are released.
 *
 * Every PARCLog instance has a logging level, a threshold that is set via `parcLog_SetLevel`
 * that determines what kind of PARCLogEntry instances are actually logged.
 * The PARCLogLevel PARCLogLevel_Emergency is always logged regardless of the current logging level.
 *
 */
#ifndef libparc_parc_Logger_h
#define libparc_parc_Logger_h

#include <stdarg.h>

#include <parc/logging/parc_LogReporter.h>
#include <parc/logging/parc_LogEntry.h>
#include <parc/logging/parc_LogLevel.h>

struct PARCLog;
typedef struct PARCLog PARCLog;

/**
 * Create a valid PARCLog instance.
 *
 * The initial instance's log level is set to `PARCLogLevel_Off`.
 *
 * @param [in] hostName A pointer to a nul-terminated C string, or NULL (See {@link PARCLogEntry}).
 * @param [in] applicationName A pointer to a nul-terminated C string, or NULL (See {@link PARCLogEntry}).
 * @param [in] processId A pointer to a nul-terminated C string, or NULL (See {@link PARCLogEntry}).
 * @param [in] reporter A pointer to a valid `PARCLogReporter` instance.
 *
 * @return non-NULL A valid PARCLog instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *     parcLogReporter_Release(&reporter);
 * }
 * @endcode
 */
PARCLog *parcLog_Create(const char *hostName, const char *applicationName, const char *processId, PARCLogReporter *reporter);

/**
 * Increase the number of references to a `PARCLog`.
 *
 * Note that new `PARCLog` is not created,
 * only that the given `PARCLog` reference count is incremented.
 * Discard the reference by invoking `parcLog_Release`.
 *
 * @param [in] parcLog A pointer to a `PARCLog` instance.
 *
 * @return The input `PARCLog` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *
 *     PARCLog *x_2 = parcLog_Acquire(log);
 *
 *     parcLog_Release(&log);
 *     parcLog_Release(&x_2);
 * }
 * @endcode
 */
PARCLog *parcLog_Acquire(const PARCLog *parcLog);

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
 * @param [in,out] logPtr A pointer to a PARCLog instance pointer, which will be set to zero on return.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *     parcLogReporter_Release(&reporter);
 *
 *     parcLog_Release(&log);
 * }
 * @endcode
 */
void parcLog_Release(PARCLog **logPtr);

/**
 * Set the log severity level to the given value.
 *
 * The level is the maximum severity that will be logged via the PARCLogReporter.
 * The log severity PARCLogLevel_Emergency cannot be blocked.
 *
 * @param [in] log A pointer to valid instance of PARCLog.
 * @param [in] level A pointer to valid instance of PARCLogLevel.
 *
 * @return The previous value of the threshold.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *     parcLogReporter_Release(&reporter);
 *
 *     PARCLogLevel old = parcLog_SetLevel(log, PARCLogLevel_Warning);
 *
 *     parcLog_SetLevel(log, old);
 *
 *     parcLog_Release(&log);
 * }
 * @endcode
 */
PARCLogLevel parcLog_SetLevel(PARCLog *log, const PARCLogLevel level);

/**
 * Get the severity level of the given PARCLog instance.
 *
 * The level is the maximum severity that will be logged via the PARCLogReporter.
 * The log severity PARCLogLevel_Emergency cannot be blocked.
 *
 * @param [in] log A pointer to valid instance of PARCLog.
 *
 * @return The severity level of the given PARCLog instance.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *     parcLogReporter_Release(&reporter);
 *
 *     PARCLogLevel level = parcLog_GetLevel(log, PARCLogLevel_Warning);
 *
 *     parcLog_Release(&log);
 * }
 * @endcode
 */
PARCLogLevel parcLog_GetLevel(const PARCLog *log);

/**
 * Test if a PARCLogLevel would be logged by the current state of the given PARCLog instance.
 *
 * @param [in] log A pointer to valid instance of PARCLog.
 * @param [in] level An instance of PARCLogLevel.
 *
 * @return true A PARCLogEntry of the given level would be logged.
 * @return false A PARCLogEntry of the given level would be logged.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
 *
 *     PARCLogReporter *reporter = parcLogReporterFile_Create(output);
 *
 *     parcOutputStream_Release(&output);
 *     parcFileOutputStream_Release(&fileOutput);
 *
 *     PARCLog *log = parcLog_Create("localhost", "myApp", "daemon", reporter);
 *     parcLogReporter_Release(&reporter);
 *
 *     if (parcLog_IsLoggable(log, PARCLogLevel_Warning)) {
 *         printf("Logging is set to Warning severity level\n");
 *     }
 *
 *     parcLog_Release(&log);
 * }
 * @endcode
 */

#define parcLog_IsLoggable(_log_, _level_) \
    (_level_ == PARCLogLevel_Emergency) || (parcLogLevel_Compare(parcLog_GetLevel(_log_), _level_) >= 0)
//bool parcLog_IsLoggable(const PARCLog *log, const PARCLogLevel level);

/**
 * Compose and emit a log message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] level An instance of PARCLogLevel.
 * @param [in] messageId A value for the message identifier.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ap A `va_list` representing the parameters for the format specification.
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower than the specified PARCLogLevel.
 *
 * Example:
 * @code
 *     parcLog_MessageVaList(log, PARCLogLevel_Warning, 123, "This is a warning message.");
 * @endcode
 */
bool parcLog_MessageVaList(PARCLog *log, PARCLogLevel level, uint64_t messageId, const char *format, va_list ap);

/**
 * Compose and emit a log message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] level An instance of PARCLogLevel.
 * @param [in] messageId A value for the message identifier.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower than the specified PARCLogLevel.
 *
 * Example:
 * @code
 *     parcLog_Message(log, PARCLogLevel_Warning, "This is a warning message.");
 * @endcode
 */
bool parcLog_Message(PARCLog *log, PARCLogLevel level, uint64_t messageId, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Warning message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_Warning(log, "This is a warning message.");
 * @endcode
 */
bool parcLog_Warning(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Message level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_Info(log, "This is an info message.");
 * @endcode
 */
bool parcLog_Info(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Notice level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_Notice(log, "This is a notice message.");
 * @endcode
 */
bool parcLog_Notice(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Debug level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_DebugMessage(log, "This is a debug message.");
 * @endcode
 */
bool parcLog_Debug(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Error level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_ErrorMessage(log, "This is an error message.");
 * @endcode
 */
bool parcLog_Error(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Critical level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_CriticalMessage(log, "This is a critical message.");
 * @endcode
 */
bool parcLog_Critical(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Alert level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_AlertMessage(log, "This is an alert message.");
 * @endcode
 */
bool parcLog_Alert(PARCLog *log, const char *restrict format, ...);

/**
 * Compose and emit a PARCLogLevel_Emergency level message.
 *
 * @param [in] log A pointer to a valid PARCLog instance.
 * @param [in] format A pointer to a nul-terminated C string containing a printf format specification.
 * @param [in] ... Zero or more parameters as input for the format specification).
 *
 * @return true The message was logged.
 * @return false The message was not logged because the log severity threshold level is lower.
 *
 * Example:
 * @code
 *     parcLog_EmergencyMessage(log, "This is an emergency message.");
 * @endcode
 */
bool parcLog_Emergency(PARCLog *log, const char *restrict format, ...);
#endif // libparc_parc_Logger_h
