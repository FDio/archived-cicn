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
 * @file parc_HashCode.h
 * @ingroup object
 * @brief The type returned from implementations of the _HashCode() function.
 *
 * The size of a PARCHashCode value may be different depending on the compile-time compilation environment.
 *
 */
#ifndef PARC_Library_parc_HashCode_h
#define PARC_Library_parc_HashCode_h

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#define PARCHashCodeSize 64
//#define PARCHashCodeSize 32

/**
 * @typedef PARCHashCode
 * @brief The type returned from implementations of the _HashCode() function.
 */
#if PARCHashCodeSize == 64
#define PRIPARCHashCode PRIu64
#define PRIXPARCHashCode PRIX64
#define PRIxPARCHashCode PRIx64
typedef uint64_t PARCHashCode;

#else
#define PRIPARCHashCode PRIu32
#define PRIXPARCHashCode PRIX32
#define PRIxPARCHashCode PRIx32
typedef uint32_t PARCHashCode;

#endif

extern const PARCHashCode parcHashCode_InitialValue;

#define parcHashCode_Hash(_memory_, _length_) parcHashCode_HashImpl(_memory_, _length_, parcHashCode_InitialValue)

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] memory A pointer to bytes used to generate the `PARCHashCode`.
 * @param [in] length The number of bytes in memory to use to generate the `PARCHashCode`
 * @param [in] initialValue An inital value for the `PARCHashCode`.
 *
 * @return The resulting `PARCHashCode` value.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCHashCode parcHashCode_HashImpl(const uint8_t *memory, size_t length, PARCHashCode initialValue);

/**
 * Hash a PARcHashCode into an existing PARCHashCode.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] initialValue The PARCHashCode initial value
 * @param [in] update The PARCHashCode value to update the initial value.
 *
 * @return The updated PARCHashCode value
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCHashCode parcHashCode_HashHashCode(PARCHashCode initialValue, PARCHashCode update);
#endif
