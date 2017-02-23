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
 * @file rta_CommandTransmitStatistics.h
 * @brief Represents a command to setup a statistics file
 *
 * Used to construct an RtaCommand object that is passed to rtaTransport_PassCommand() or _rtaTransport_SendCommandToFramework()
 * to send a command from the API's thread of execution to the Transport's thread of execution.
 *
 */
#ifndef Libccnx_rta_CommandTransmitStatistics_h
#define Libccnx_rta_CommandTransmitStatistics_h

struct rta_command_transmitstatistics;
typedef struct rta_command_transmitstatistics RtaCommandTransmitStatistics;

RtaCommandTransmitStatistics *rtaCommandTransmitStatistics_Create(struct timeval period, const char *filename);

/**
 * Increase the number of references to a `RtaCommandTransmitStatistics`.
 *
 * Note that new `RtaCommandTransmitStatistics` is not created,
 * only that the given `RtaCommandTransmitStatistics` reference count is incremented.
 * Discard the reference by invoking `rtaCommandTransmitStatistics_Release`.
 *
 * @param [in] transmitStats The RtaCommandTransmitStatistics to reference.
 *
 * @return non-null A reference to `transmitStats`.
 * @return null An error
 *
 * Example:
 * @code
 * {
 *    RtaCommandOpenConnection *transmitStats = rtaCommandTransmitStatistics_Create((struct timeval) { 1, 2 }, "filename");
 *    RtaCommandOpenConnection *second = rtaCommandTransmitStatistics_Acquire(transmitStats);
 *
 *    // release order does not matter
 *    rtaCommandTransmitStatistics_Release(&transmitStats);
 *    rtaCommandTransmitStatistics_Release(&second);
 * }
 * @endcode
 */
RtaCommandTransmitStatistics *rtaCommandTransmitStatistics_Acquire(const RtaCommandTransmitStatistics *transmitStats);

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
 * @param [in,out] openPtr A pointer to the object to release, will return NULL'd.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
void rtaCommandTransmitStatistics_Release(RtaCommandTransmitStatistics **openPtr);

/**
 * Returns the time period to use when writing statistics
 *
 * The time period is how often the transport will write the statistics to the specified file.
 *
 * @param [in] transmitStats An allocated RtaCommandTransmitStatistics
 *
 * @return timeval The value passed to rtaCommandTransmitStatistics_Create().
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *     struct timeval period = { 1, 2 };
 *     const char *filename = "filename";
 *     RtaCommandOpenConnection *transmitStats = rtaCommandTransmitStatistics_Create(period, filename);
 *     struct timeval testValue = rtaCommandTransmitStatistics_GetPeriod(transmitStats);
 *     assertTrue(timercmp(&testValue, &period, ==), "Wrong period");
 *     rtaCommandTransmitStatistics_Release(&transmitStats);
 * }
 * @endcode
 */
struct timeval rtaCommandTransmitStatistics_GetPeriod(const RtaCommandTransmitStatistics *transmitStats);

/**
 * Returns the filename to use when writing statistics
 *
 * The filename to append statistics to.
 *
 * @param [in] transmitStats An allocated RtaCommandTransmitStatistics
 *
 * @return timeval The value passed to rtaCommandTransmitStatistics_Create().
 *
 * Example:
 * @code
 * {
 *     int stackId = 7;
 *     struct timeval period = { 1, 2 };
 *     const char *filename = "filename";
 *     RtaCommandOpenConnection *transmitStats = rtaCommandTransmitStatistics_Create(period, filename);
 *     struct timeval testValue = rtaCommandTransmitStatistics_GetPeriod(transmitStats);
 *     assertTrue(strcmp(filename, testValue) == 0, "Wrong filename");
 *     rtaCommandTransmitStatistics_Release(&transmitStats);
 * }
 * @endcode
 */
const char *rtaCommandTransmitStatistics_GetFilename(const RtaCommandTransmitStatistics *transmitStats);
#endif // Libccnx_rta_CommandTransmitStatistics_h
