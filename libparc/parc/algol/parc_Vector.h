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
 * @file parc_Vector.h
 * @ingroup datastructures
 * @brief PARC Vector
 * A Vector consists of a pointer and associated length in bytes of data in memory.
 *
 *
 */
#ifndef libparc_parc_Vector_h
#define libparc_parc_Vector_h

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct parc_vector;

/**
 * A `PARCVector` is a tuple consisting of an address and a length.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef struct parc_vector PARCVector;

/**
 * Create a new `PARCVector` consisting of a pointer and a length.
 *
 * @param [in] pointer A pointer to memory.
 * @param [in] length The length, in bytes.
 * @return An allocated `PARCVector` structure.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCVector *parcVector_Create(const void *pointer, const size_t length);

/**
 * Initialise the given `PARCVector` to the given values.
 *
 * @param [in,out] vector A pointer to the instance of `PARCVector` to initialize
 * @param [in] pointer A pointer to the memory with which to initialize @p vector.
 * @param [in] length The length of the @p pointer contents.
 * @return The pointer to the input @p vector.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCVector *parcVector_Init(PARCVector *vector, const void *pointer, const size_t length);

/**
 * Get the memory pointer for this `PARCVector`.
 *
 * @param [in] vector A pointer to the instance of `PARCVector` from which to get the memory pointer.
 * @return The memory pointer of @p vector
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const void *parcVector_GetPointer(const PARCVector *vector);

/**
 * Get the length of of the `PARCVector`
 *
 * @param [in] vector The `PARCVector` instance of interest.
 * @return The length of @p vector.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcVector_GetLength(const PARCVector *vector);

/**
 * Destroy the instance of `PARCVector`
 *
 * @param [in,out] vector The pointer to the pointer to the `PARCVector` instance to destroy.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcVector_Destroy(PARCVector **vector);
#endif // libparc_parc_Vector_h
