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
 * @file parc_Time.h
 * @ingroup inputoutput
 * @brief Time Manipulation
 *
 * Different platforms have different ways to express time-of-day, elapsed-time, clock-time.
 * In some cases multiple ways to express the same semantic value have evolved over time,
 * for example `struct timeval` and `struct timespec`.
 *
 */
#ifndef libparc_parc_Time_h
#define libparc_parc_Time_h

#include <sys/time.h>
#include <stdint.h>

/**
 * Create a nul-terminated C string containing the formatted representation of a `struct timeval`.
 *
 * The minimum length of the result is 8 characters consisting of the `struct timeval`
 * formatted as a decimal string consisting of the number of seconds since midnight (0 hour), January 1, 1970.
 *
 * @param [in] timeval The instance of the struct `timeval` to convert to a C string.
 * @return An allocated, null-terminated C string that must be freed via {@link parcMemory_Deallocate()}.
 *
 * Example:
 * @code
 * {
 *     struct timeval timeval;
 *     gettimeofday(&timeval, NULL);
 *
 *     char *string = parcTime_TimevalAsString(timeval);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 */
char *parcTime_TimevalAsString(struct timeval timeval);

/**
 * Format an ISO8601 date from the given struct timeval into the given character array, @p result.
 *
 * @param [in] utcTime A pointer to a valid struct timeval instance with the time in UTC.
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     struct timeval theTime;
 *     gettimeofday(&theTime, NULL);
 *
 *     char result[64];
 *     parcTime_TimevalAsISO8601(&theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_TimevalAsISO8601(const struct timeval *utcTime, char *result);

/**
 * Format an RFC3339 compliant date from the given struct timeval into the given character array.
 *
 * @param [in] utcTime A pointer to a valid struct timeval instance with the time in UTC.
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     struct timeval theTime;
 *     gettimeofday(&theTime, NULL);
 *
 *     char result[64];
 *     parcTime_TimevalAsRFC3339(&theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_TimevalAsRFC3339(const struct timeval *utcTime, char *result);

/**
 * Format an ISO8601 date from the given `time_t` value into the given character array, @p result.
 *
 * @param [in] utcTime `time_t` value representing the time in UTC.
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     time_t theTime = time(0);
 *
 *     char result[64];
 *     parcTime_TimeAsISO8601(theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_TimeAsISO8601(const time_t utcTime, char *result);

/**
 * Format the current time as an ISO8601 date into the given character array, @p result.
 *
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     char result[64];
 *     parcTime_NowAsISO8601(theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_NowAsISO8601(char *result);

/**
 * Format an RFC3339 compliant date from the given `time_t` value into the given character array.
 *
 * @param [in] utcTime `time_t` value of the time in UTC.
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     struct timeval theTime;
 *     time_t theTime = time(0);
 *
 *     char result[64];
 *     parcTime_TimeAsRFC3339(&theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_TimeAsRFC3339(const time_t utcTime, char *result);

/**
 * Format the current time as an RFC3339 compliant date into the given character array.
 *
 * @param [out] result A pointer to the 64 element nul-terminated character array, @p result.
 *
 * @return The same value as @p result.
 *
 * Example:
 * @code
 * {
 *     char result[64];
 *     parcTime_NowAsRFC3339(&theTime, result);
 *
 * }
 * @endcode
 */
char *parcTime_NowAsRFC3339(char *result);

/**
 * Add two `struct timeval` values together.
 *
 * @param [in] addend1 The first value.
 * @param [in] addend2 The second value.
 *
 * @return The sum of the first and second values.
 *
 * Example:
 * @code
 * {
 *     struct timeval now;
 *     gettimeofday(&now, NULL);
 *
 *     struct timeval theEnd = parcTime_TimevalAdd(&now, timeout);
 * }
 * @endcode
 */
struct timeval parcTime_TimevalAdd(const struct timeval *addend1, const struct timeval *addend2);

/**
 * Subtract two `struct timeval` values.
 *
 * @param [in] minuend The number from which the subtrahend is to be subtracted.
 * @param [in] subtrahend The subtrahend.
 *
 * @return The difference between the first and second values.
 *
 * Example:
 * @code
 * {
 *     struct timeval now;
 *     gettimeofday(&now, NULL);
 *
 *     struct timeval result = parcTime_TimevalSubtract(&now, timeout);
 * }
 * @endcode
 */
struct timeval parcTime_TimevalSubtract(const struct timeval *minuend, const struct timeval *subtrahend);

/**
 * The current time as a `struct timeval`.
 *
 * @return The current time as a `struct timeval`.
 *
 * Example:
 * @code
 * {
 *
 *     struct timeval now = parcTime_NowTimeval();
 * }
 * @endcode
 */
struct timeval parcTime_NowTimeval(void);

/**
 * The current time in microseconds since midnight (0 hour), January 1, 1970 as a `uint64_t`.
 *
 * @return The current time in microseconds since midnight (0 hour), January 1, 1970 as a `uint64_t`.
 *
 * Example:
 * @code
 * {
 *
 *     uint64_t now = parcTime_NowMicroseconds();
 * }
 * @endcode
 */
uint64_t parcTime_NowMicroseconds(void);

/**
 * The current time in nanoseconds since midnight (0 hour), January 1, 1970 as a `uint64_t`.
 *
 * @return The current time in nanoseconds since midnight (0 hour), January 1, 1970 as a `uint64_t`.
 *
 * Example:
 * @code
 * {
 *
 *     uint64_t now = parcTime_NowNanoseconds();
 * }
 * @endcode
 */
uint64_t parcTime_NowNanoseconds(void);
#endif // libparc_parc_Time_h
