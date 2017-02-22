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

#ifndef ccnxPing_Stats_h
#define ccnxPing_Stats_h

/**
 * Structure to collect and display the performance statistics.
 */
struct ping_stats;
typedef struct ping_stats CCNxPingStats;

/**
 * Create an empty `CCNxPingStats` instance.
 *
 * The returned result must be freed via {@link ccnxPingStats_Release}
 *
 * @return A newly allocated `CCNxPingStats`.
 *
 * Example
 * @code
 * {
 *     CCNxPingStats *stats = ccnxPingStats_Create();
 * }
 * @endcode
 */
CCNxPingStats *ccnxPingStats_Create(void);

/**
 * Increase the number of references to a `CCNxPingStats`.
 *
 * Note that new `CCNxPingStats` is not created,
 * only that the given `CCNxPingStats` reference count is incremented.
 * Discard the reference by invoking `ccnxPingStats_Release`.
 *
 * @param [in] clock A pointer to a `CCNxPingStats` instance.
 *
 * @return The input `CCNxPingStats` pointer.
 *
 * Example:
 * @code
 * {
 *     CCNxPingStats *stats = ccnxPingStats_Create();
 *     CCNxPingStats *copy = ccnxPingStats_Acquire(stats);
 *     ccnxPingStats_Release(&stats);
 *     ccnxPingStats_Release(&copy);
 * }
 * @endcode
 */
CCNxPingStats *ccnxPingStats_Acquire(const CCNxPingStats *stats);

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
 * @param [in,out] clockPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxPingStats *stats = ccnxPingStats_Create();
 *     CCNxPingStats *copy = ccnxPingStats_Acquire(stats);
 *     ccnxPingStats_Release(&stats);
 *     ccnxPingStats_Release(&copy);
 * }
 * @endcode
 */
void ccnxPingStats_Release(CCNxPingStats **statsPtr);

/**
 * Record the name and time for a request (e.g., interest).
 *
 * @param [in] stats The `CCNxPingStats` instance.
 * @param [in] name The `CCNxName` name structure.
 * @param [in] timeInUs The send time (in microseconds).
 */
void ccnxPingStats_RecordRequest(CCNxPingStats *stats, CCNxName *name, uint64_t timeInUs);

/**
 * Record the name and time for a response (e.g., content object).
 *
 * @param [in] stats The `CCNxPingStats` instance.
 * @param [in] name The `CCNxName` name structure.
 * @param [in] timeInUs The send time (in microseconds).
 * @param [in] message The response `CCNxMetaMessage`.
 *
 * @return The delta between the request and response (in microseconds).
 */
uint64_t ccnxPingStats_RecordResponse(CCNxPingStats *stats,
                                      CCNxName *name,
                                      uint64_t timeInUs,
                                      CCNxMetaMessage *message,
                                      bool *existing);

size_t ccnxPingStats_RecordLost(CCNxPingStats *stats, CCNxName *nameResponse);

/**
 * Display the average statistics stored in this `CCNxPingStats` instance.
 *
 * @param [in] stats The `CCNxPingStats` instance from which to draw the average data.
 *
 * @retval true If the stats were displayed correctly
 * @retval false Otherwise
 */
bool ccnxPingStats_Display(CCNxPingStats *stats);
#endif // ccnxPing_Stats_h
