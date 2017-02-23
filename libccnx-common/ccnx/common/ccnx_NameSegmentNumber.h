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
 * @file ccnx_NameSegmentNumber.h
 * @ingroup Naming
 * @brief Utilities for creating and parsing CCNxNameSegments that contain an integer value.
 *
 * CCNxNameSegments may optionally contain an integer value. This file contains some utilities
 * for creating and parsing CCNxNameSegments with integer values.
 * See {@link CCNxNameSegment}, {@link CCNxNameLabelType}, and {@link CCNxName} for more information.
 *
 */
#ifndef libccnx_ccnx_NameSegmentNumber_h
#define libccnx_ccnx_NameSegmentNumber_h

#include <stdbool.h>

#include <ccnx/common/ccnx_NameSegment.h>
#include <ccnx/common/ccnx_NameLabel.h>

/**
 * Create a new {@link CCNxNameSegment} consisting of a type and integer value.
 *
 * The newly created instance must eventually be released by calling {@link ccnxNameSegment_Release}().
 *
 * @param [in] type A valid {@link CCNxNameLabelType}.
 * @param [in] value The integer value to assign to the `CCNxNameSegment`.
 * @return A pointer to a new `CCNxNameSegment` instance, consisting of a type and integer value.
 *
 * Example:
 * @code
 * {
 *     uint64_t expected = 0x123456789ABCDEF0;
 *     CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);
 *
 *     uint64_t actual = ccnxNameSegmentNumber_Value(segment);
 *
 *     assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
 *
 *     ccnxNameSegment_Release(&segment);
 * }
 * @endcode
 *
 * @see {@link ccnxNameSegment_Release}
 * @see `CCNxNameLabelType`
 */
CCNxNameSegment *ccnxNameSegmentNumber_Create(CCNxNameLabelType type, uint64_t value);

/**
 * Decode an integer value from a {@link CCNxNameSegment}.
 *
 * Given a `CCNxNameSegment` with a numeric type, return the integer value associated with the `CCNxNameSegment`.
 *
 * @param [in] nameSegment A pointer a `CCNxNameSegment` instance containing an integer value.
 * @return The integer value of the `CCNxNameSegment`.
 *
 * Example:
 * @code
 * {
 *     uint64_t expected = 0x123456789ABCDEF0;
 *     CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);
 *
 *     uint64_t actual = ccnxNameSegmentNumber_Value(segment);
 *
 *     assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
 *
 *     ccnxNameSegment_Release(&segment);
 * }
 * @endcode
 */
uint64_t ccnxNameSegmentNumber_Value(const CCNxNameSegment *nameSegment);

/**
 * Determine if the given CCNxNameSegment value represents a valid encoded number.
 *
 * A valid encoded number contains at least 1 byte and nor more than 8 bytes.
 *
 * @param [in] nameSegment A pointer to a CCNxNameSegment instance.
 *
 * Example:
 * @code
 * {
 *     uint64_t expected = 0x123456789ABCDEF0;
 *     CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);
 *
 *     ccnxNameSegmentNumber_AssertValid(segment);
 * }
 * @endcode
 */
bool ccnxNameSegmentNumber_IsValid(const CCNxNameSegment *nameSegment);

/**
 * Assert that the given CCNxNameSegment value represents a valid encoded number.
 *
 * @param [in] nameSegment A pointer to a CCNxNameSegment instance.
 *
 * Example:
 * @code
 * {
 *     uint64_t expected = 0x123456789ABCDEF0;
 *     CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);
 *
 *     ccnxNameSegmentNumber_AssertValid(segment);
 * }
 * @endcode
 */
void ccnxNameSegmentNumber_AssertValid(const CCNxNameSegment *nameSegment);
#endif // libccnx_ccnx_NameSegmentNumber_h
