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
 * @file ccnx_TimeStamp.h
 * @brief A CCNxTimeStamp represents a point in time.
 *
 */
#ifndef libccnx_ccnx_TimeStamp_h
#define libccnx_ccnx_TimeStamp_h

#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>

struct ccnx_timestamp;
/**
 * @typedef CCNxTimeStamp
 * @brief A CCNxTimeStamp represents a point in time.
 */
typedef struct ccnx_timestamp CCNxTimeStamp;

/**
 * Assert that a pointer to a `CCNxTimeStamp` instance and the instance itself is valid.
 *
 * @param [in] timeStamp A pointer to a CCNxTimeStamp intance.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timestamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
 *     ccnxTimeStamp_AssertValid(timestamp);
 *     ccnxTimeStamp_Release(&timestamp);
 * }
 * @endcode
 *
 */
void ccnxTimeStamp_AssertValid(const CCNxTimeStamp *timeStamp);

/**
 * Create a new `CCNxTimeStamp` instance from the current local time.
 *
 * The newly created instance must eventually be released by calling {@link ccnxTimeStamp_Release}();
 *
 * @return An instance of a `CCNxTimeStamp` initialized to the current local time.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timestamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
 *     ccnxTimeStamp_AssertValid(timestamp);
 *     ccnxTimeStamp_Release(&timestamp);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
CCNxTimeStamp *ccnxTimeStamp_CreateFromCurrentUTCTime(void);

/**
 * Create a new `CCNxTimeStamp` instance from the given `struct timespec`.
 *
 * Given a `struct timespec` create a `CCNxTimeStamp` representing the same time.
 * The newly created instance must eventually be released by calling {@link ccnxTimeStamp_Release}();
 *
 * @param [in] timespec The `struct timespec` of the time to represent.
 * @return An instance of a `CCNxTimeStamp` initialized from the specified `struct timespec`.
 *
 * Example:
 * @code
 * {
 *     struct timespec time = {.tv_sec = 1, .tv_nsec = 1 };
 *
 *     CCNxTimeStamp *timestamp = ccnxTimeStamp_CreateFromTimespec(&time);
 *
 *     ccnxTimeStamp_Release(&timestamp);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
CCNxTimeStamp *ccnxTimeStamp_CreateFromTimespec(const struct timespec *timespec);

/**
 * Return a `struct timespec` representation of the given `CCNxTimeStamp`.
 *
 * The internal form of the `CCNxTimeStamp` is returned as a `struct timespec`.
 *
 * @param [in] timeStamp A pointer to a `CCNxTimeStamp` instance.
 * @return A `struct timespec` representation of the given `CCNxTimeStamp`.
 *
 * Example:
 * @code
 * {
 *     struct timespec time = {.tv_sec = 1, .tv_nsec = 1 };
 *
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromTimespec(&time);
 *
 *     struct timespec actualTime = ccnxTimeStamp_AsTimespec(timeStamp);
 *     assertTrue(time.tv_sec == actualTime.tv_sec && time.tv_nsec == actualTime.tv_nsec, "Expected timespec to be equal.");
 *
 *     ccnxTimeStamp_Release(&timeStamp);
 * }
 * @endcode
 */
struct timespec ccnxTimestamp_AsTimespec(const CCNxTimeStamp *timeStamp);

/**
 * Create a new `CCNxTimeStamp` instance initialized to the given number of milliseconds from the epoch.
 *
 * The epoch is defined as 00:00:00, 01/01/1970.
 * The newly created instance must eventually be released by calling {@link ccnxTimeStamp_Release}();
 *
 * @param [in] milliseconds A `uint64_t` specifying the number of milliseconds since the epoch.
 * @return A pointer to a `CCNxTimeStamp` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timestamp = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(100);
 *     ccnxTimeStamp_AssertValid(timestamp);
 *     ccnxTimeStamp_Release(&timestamp);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
extern CCNxTimeStamp *ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(uint64_t milliseconds);

/**
 * Create a new `CCNxTimeStamp` instance initialized to the given number of nanoseconds from the epoch.
 *
 * The epoch is defined as 00:00:00, 01/01/1970.
 * The newly created instance must eventually be released by calling {@link ccnxTimeStamp_Release}();
 *
 * @param [in] nanoseconds A uint64_t specifying the number of nanoseconds since the epoch.
 * @return A pointer to a `CCNxTimeStamp` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timestamp = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(100);
 *     ccnxTimeStamp_AssertValid(timestamp);
 *     ccnxTimeStamp_Release(&timestamp);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
CCNxTimeStamp *ccnxTimeStamp_CreateFromNanosecondsSinceEpoch(uint64_t nanoseconds);

/**
 * Return the value of the given `CCNxTimeStamp` as an unsigned 64-bit integer representing the number of nanoseconds since the epoch.
 *
 * The resolution of a `CCNxTimeStamp` is nanoseconds, although the resolution of the host environment may not.
 *
 * @param [in] timeStamp A pointer to a `CCNxTimeStamp` instance.
 * @return The value of the given `CCNxTimeStamp` as an unsigned 64-bit integer
 *
 * Example:
 * @code
 * {
 *     uint64_t expected = 1099511627776ULL;
 *
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromNanosecondsSinceEpoch(expected);
 *
 *     uint64_t actual = ccnxTimeStamp_AsNanoSeconds(timeStamp);
 *
 *     assertTrue(expected == actual, "Expected " PRIu64 " actual " PRIu64, expected, actual);
 *
 *     ccnxTimeStamp_Release(&timeStamp);
 * }
 * @endcode
 */
uint64_t ccnxTimeStamp_AsNanoSeconds(const CCNxTimeStamp *timeStamp);

/**
 * Create a deep copy of the given `CCNxTimeStamp`, using dynamically allocated memory.
 *
 * The newly created instance must eventually be released by calling {@link ccnxTimeStamp_Release}();
 *
 * @param [in] timeStamp  A pointer to a `CCNxTimeStamp` instance.
 * @return A new copy of the given `CCNxTimeStamp`.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(100);
 *     CCNxTimeStamp *copy = ccnxTimeStamp_Copy(timeStamp);
 *
 *     ccnxTimeStamp_AssertValid(timeStamp);
 *     ccnxTimeStamp_AssertValid(copy);
 *
 *     ccnxTimeStamp_Release(&timeStamp);
 *     ccnxTimeStamp_Release(&copy);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
extern CCNxTimeStamp *ccnxTimeStamp_Copy(const CCNxTimeStamp *timeStamp);

/**
 * Determine if two `CCNxTimeStamp` instances are equal.
 *
 *
 * The following equivalence relations on non-null `CCNxTimeStamp` instances are maintained: *
 * It is reflexive: for any non-null reference value `ccnxTimeStamp_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxTimeStamp_Equals(x, y)` must return true if and only if
 *        `ccnxTimeStamp_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxTimeStamp_Equals(x, y)` returns true and
 *        `ccnxTimeStamp_Equals(y, z)` returns true,
 *        then  `ccnxTimeStamp_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxTimeStamp_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxTimeStamp_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] timeStampA A pointer to a `CCNxTimeStamp` instance.
 * @param [in] timeStampB A pointer to a `CCNxTimeStamp` instance.
 * @return `true` if the referenced CCNxTimeStamp instances are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timeStampA = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(100);
 *     CCNxTimeStamp *timeStampB = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(100);
 *
 *     if (ccnxTimeStamp_Equals(timeStampA, timeStampB)) {
 *         // true
 *     } else {
 *        // false
 *     }
 *
 *     ccnxTimeStamp_Release(&timeStampA);
 *     ccnxTimeStamp_Release(&timeStampB);
 * }
 * @endcode
 */
bool ccnxTimeStamp_Equals(const CCNxTimeStamp *timeStampA, const CCNxTimeStamp *timeStampB);

/**
 * Increase the number of references to a `CCNxTimeStamp`.
 *
 * Note that new `CCNxTimeStamp` is not created,
 * only that the given `CCNxTimeStamp` reference count is incremented.
 * Discard the reference by invoking {@link ccnxTimeStamp_Release}.
 *
 * @param [in] instance A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
 *
 *     CCNxTimeStamp *reference = ccnxTimeStamp_Acquire(timeStamp);
 *
 *     ccnxTimeStamp_Release(&timeStamp);
 *     ccnxTimeStamp_Release(&reference);
 * }
 * @endcode
 *
 * @see ccnxTimeStamp_Release
 */
CCNxTimeStamp *ccnxTimeStamp_Acquire(const CCNxTimeStamp *instance);

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
 * @param [in,out] timeStampPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
 *
 *     ccnxTimeStamp_Release(&timeStamp);
 * }
 * @endcode
 *
 * @see {@link ccnxTimeStamp_Acquire}
 */
void ccnxTimeStamp_Release(CCNxTimeStamp **timeStampPtr);

/**
 * Produce a null-terminated C-string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] timeStamp A pointer to a 'CCNxTimeStamp' instance.
 * @return NULL Memory could not be allocated.
 * @return non-NULL A nul-terminated string that must be deallocated via {@link parcMemory_Deallocate}().
 *
 * Example:
 * @code
 * {
 *     CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
 *
 *     char *string = ccnxTimeStamp_ToString(timeStamp);
 *
 *     printf("Hello: %s\n", string);
 *
 *     parcMemory_Deallocate(string);
 *     ccnxTimeStamp_Release(&timeStamp);
 * }
 * @endcode
 *
 * @see `parcMemory_Deallocate`
 */
char *ccnxTimeStamp_ToString(const CCNxTimeStamp *timeStamp);
#endif // libccnx_ccnx_TimeStamp_h
