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
 * @file assertions.h
 * @ingroup runtime
 * @brief Runtime and Test Assertions
 *
 */
#ifndef LongBow_assertions_h
#define LongBow_assertions_h

#ifndef LongBow_runtime_h
#error "Do not include LongBow/assertions.h directly.  Include LongBow/runtime.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <LongBow/traps.h>
#include <LongBow/tests.h>

#include <LongBow/longBow_Location.h>
#include <LongBow/longBow_Event.h>

/**
 * @def assertTrue
 * @brief Assert that a condition is true.
 *
 * @param condition If false, the assertion triggers.
 * @param ... A printf formatting string (required) and parameters (optional).
 *
 * Example:
 * @code
 * {
 *     assertTrue(2 + 2 == 4, "Adding 2 + 2 did not equal 4.");
 * }
 * @endcode
 */
#define assertTrue(condition, ...) longBowAssert(&LongBowAssertEvent, (condition), __VA_ARGS__)

/**
 * @def assertFalse
 * @brief Assert that a condition is false.
 *
 * @param condition If true, the assertion triggers.
 * @param ... A printf formatting string (required) and parameters (optional).
 * Example:
 * @code
 * {
 *     assertFalse(2 + 2 == 5, "Adding 2 + 2 must not equal 5.");
 * }
 * @endcode
 */
#define assertFalse(condition, ...) longBowAssert(&LongBowAssertEvent, !(condition), __VA_ARGS__)

/**
 * @def assertNotNull
 * @brief Assert that the given value is not NULL.
 *
 * @param x     If the value is NULL, the assertion triggers.
 * @param ...   A printf formatting string (required) and parameters (optional).
 * Example:
 * @code
 * void
 * function(char *p)
 * {
 *     assertNotNull(p, "Parameter must not be NULL");
 * }
 * @endcode
 */
#define assertNotNull(x, ...) longBowAssert(&LongBowAssertEvent, (x) != NULL, __VA_ARGS__)

/**
 * @def assertNull
 * @brief Assert that the given value is NULL.
 *
 * @param x The value to test.
 * @param ... A printf formatting string (required) and parameters (optional).
 * Example:
 * @code
 * void
 * function(char *p)
 * {
 *     assertNull(p, "Parameter must be NULL");
 * }
 * @endcode
 */
#define assertNull(x, ...) longBowAssert(&LongBowAssertEvent, (x) == NULL, __VA_ARGS__)

/**
 * @def assertAligned
 * @brief  Assert that the given address is aligned according to `alignment`.
 *
 * Return true of the given address is aligned according to alignment.
 * The value for alignment must be a power of 2.
 *
 * @param address A pointer to memory
 * @param alignment A power of 2 representing the memory alignment of address.
 * @param ... A printf formatting string (required) and parameters (optional).
 *
 * Example:
 * @code
 * void
 * function(void *p)
 * {
 *     assertAligned(p, 4, "Parameter must be aligned on 4 byte boundaries");
 * }
 * @endcode
 */
#define assertAligned(address, alignment, ...) \
    longBowAssert(&LongBowAssertEvent, longBowRuntime_TestAddressIsAligned((address), (alignment)), __VA_ARGS__)

/**
 * @def assertEqualStrings
 * @brief Assert that two strings are equal.
 *
 * @param expected
 * @param actual
 * @param ... A printf formatting string (required) and parameters (optional).
 *
 * Example:
 * @code
 * {
 *     assertEqualStrings("hello", "world");
 * }
 * @endcode
 */
#define assertEqualStrings(expected, actual) \
    longBowAssert(&LongBowAssertEvent, strcmp((expected), (actual)) == 0, "Expected '%s' actual '%s'", (expected), (actual))

/**
 * @def assertEqual
 * @brief Assert that two values are equal, using a canonical string message.
 */
#define assertEqual(expected, actual, format) \
    assertTrue((expected) == (actual), "Expected=" format " actual=" format, (expected), (actual))
#endif /* ASSERTIONS_H_ */

