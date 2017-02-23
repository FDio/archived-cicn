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
 * @file longBow_Location.h
 * @ingroup internals
 * @brief LongBow Source File Location
 *
 * LongBow records events during execution and insofar that it's possible,
 * it records the source code location information for reporting.
 *
 */
#ifndef LongBow_longBow_Location_h
#define LongBow_longBow_Location_h

#include <stdint.h>

struct longbow_location;
/**
 * @typedef LongBowLocation
 */
typedef struct longbow_location LongBowLocation;

/**
 * Create a new LongBow location within a source code file.
 *
 * @param [in] fileName The file target of the location.
 * @param [in] functionName The function target of the location.
 * @param [in] lineNumber The exact line number within the target file.
 *
 * @return A pointer to an allocated LongBowLocation instance that must be deallocated via `longBowLocation_Destroy()`.
 *
 * Example:
 * @code
 * {
 *     LongBowLocation *location = longBowLocation_Create("test.c", "main", 101);
 * }
 * @endcode
 */
LongBowLocation *longBowLocation_Create(const char *fileName, const char *functionName, uint32_t lineNumber);

/**
 * Destroy the `LongBowLocation` instance.
 *
 * @param [in,out] locationPtr A pointer to the `LongBowLocation` instance to be destroyed.
 *
 * Example:
 * @code
 * {
 *     LongBowLocation *location = longBowLocation_Create("test.c", "main", 101);
 *     ...
 *     longBowLocation_Destroy(&location);
 * }
 * @endcode
 */
void longBowLocation_Destroy(LongBowLocation **locationPtr);

/**
 * Create a human readable representation of the `LongBowLocation` instance.
 *
 * @param [in] location The `LongBowLocation` instance from which to generate the string representation.
 *
 * @return An allocated, null-terminated C string that must be freed via free().
 *
 * Example:
 * @code
 * {
 *     LongBowLocation *location = longBowLocation_Create("test.c", "main", 101);
 *     char *stringRep = longBowLocation_ToString(location);
 *     ...
 *     longBowLocation_Destroy(&location);
 * }
 * @endcode
 */
char *longBowLocation_ToString(const LongBowLocation *location);
#endif // LongBow_longBow_Location_h
