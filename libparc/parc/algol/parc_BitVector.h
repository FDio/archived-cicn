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
 * @file parc_BitVector.h
 * @ingroup types
 *
 */
#ifndef libparc_parc_BitVector_h
#define libparc_parc_BitVector_h

#include <stdbool.h>

/**
 * @typedef PARCBitVector
 * @brief A structure containing private bit vector state data variables
 */
struct PARCBitVector;
typedef struct PARCBitVector PARCBitVector;

/**
 * Create a new bit vector instance.
 *
 * @returns NULL on error, pointer to new vector on success.
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_Create(void);

/**
 * Create a copy of a bit vector instance.
 *
 * @param [in] parcBitVector to duplicate
 * @returns NULL on error, pointer to new copy on success.
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     PARCBitVector *copy = parcBitVector_Copy(parcBitVector);
 *     assertTrue(parcBitVector_Equals(parcBitVector, copy), "Duplicate vector is unequal");
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_Copy(const PARCBitVector *parcBitVector);

/**
 * Obtain a reference to a bit vector instance.
 *
 * @param [in] parcBitVector to obtain reference to
 * @returns pointer to BitVector
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     PARCBitVector *reference = parcBitVector_Acquire(parcBitVector);
 *     parcBitVector_Release(&reference);
 *     parcBitVector_Release(&parcBitVector);
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_Acquire(const PARCBitVector *parcBitVector);

/**
 * Release a reference to a bit vector instance.
 *
 * @param [in] parcBitVector to release reference to
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     PARCBitVector *reference = parcBitVector_Acquire(parcBitVector);
 *     parcBitVector_Release(&reference);
 *     parcBitVector_Release(&parcBitVector);
 * }
 * @endcode
 *
 */
void parcBitVector_Release(PARCBitVector **parcBitVector);

/**
 * Determine equality of a pair of bit vectors
 *
 * @param [in] a bit vector to compare
 * @param [in] b bit vector to compare
 * @returns true if equal, false if unequal
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     PARCBitVector *copy = parcBitVector_Copy(parcBitVector);
 *     parcBitVector_Set(copy, 1);
 *     assertTrue(parcBitVector_Equals(parcBitVector, copy) == false, "Vector should have been unequal");
 * }
 * @endcode
 *
 */
bool parcBitVector_Equals(const PARCBitVector *a, const PARCBitVector *b);


/**
 * Determine equality of a pair of bit vectors
 *
 * @param [in] parcBitVector bit vector to search
 * @param [in] testVector bit vector to test
 * @returns true if parcBitVector contains all bits in testVector, false otherwise
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *superSet = parcBitVector_Create();
 *     parcBitVector_Set(superSet, 10);
 *     parcBitVector_Set(superSet, 11);
 *     PARCBitVector *subSet = parcBitVector_Create();
 *     parcBitVector_Set(subSet, 10);
 *     assertTrue(parcBitVector_Contains(superSet, subSet), "Expect superSet to contain subSet");
 * }
 * @endcode
 *
 */
bool parcBitVector_Contains(const PARCBitVector *parcBitVector, const PARCBitVector *testVector);

/**
 * Get the current value of a bit in a vector
 *
 * @param [in] parcBitVector to obtain value from
 * @param [in] bit in vector to get value of
 * @returns value of bit in vector, 1 or 0
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 1, "Vector should have been set");
 * }
 * @endcode
 *
 */
int parcBitVector_Get(const PARCBitVector *parcBitVector, unsigned bit);

/**
 * Set a bit in a vector
 *
 * @param [in] parcBitVector to set bit in
 * @param [in] bit in vector to set
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 1, "Vector should have been set");
 * }
 * @endcode
 *
 */
void parcBitVector_Set(PARCBitVector *parcBitVector, unsigned bit);

/**
 * Right shift a vector contents
 *
 * @param [in] parcBitVector
 * @param [in] rightShift count
 * @param [out] input vector
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 0);
 *
 *     parcBitVector_RightShift(parcBitVector, 1);
 *     assertTrue(parcBitVector_FirstBitSet(parcBitVector) == 1,
 *                "First vector element should have moved up");
 *
 *     parcBitVector_Release(&parcBitVector);
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_RightShift(PARCBitVector *parcBitVector, size_t rightShift);

/**
 * Left shift a vector contents
 *
 * @param [in] parcBitVector
 * @param [in] leftShift count
 * @param [out] input vector
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     parcBitVector_Set(parcBitVector, 0);
 *
 *     parcBitVector_LeftShift(parcBitVector, 1);
 *     assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1,
 *                "First vector element should have rolled off");
 *
 *     parcBitVector_Release(&parcBitVector);
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_LeftShift(PARCBitVector *parcBitVector, size_t leftShift);

/**
 * Logical And of a vector contents with another vector
 *
 * @param [in] a vector
 * @param [in] b vector
 * @param [out] allocated vector with result of And operation
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     PARCBitVector *andVector = parcBitVector_Create();
 *     parcBitVector_Set(andVector, 11);
 *
 *     PARCBitVector *result = parcBitVector_And(parcBitVector, andVector);
 *     assertTrue(parcBitVector_NumberOfBitsSet(result) == 0, "Vector should have been empty");
 *
 *     parcBitVector_Release(&parcBitVector);
 *     parcBitVector_Release(&andVector);
 *     parcBitVector_Release(&result);
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_And(const PARCBitVector *a, const PARCBitVector *b);

/**
 * Logical Or of a vector contents with another vector
 *
 * @param [in] a vector
 * @param [in] b vector
 * @param [out] allocated vector with result of Or operation
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     PARCBitVector *orVector = parcBitVector_Create();
 *     parcBitVector_Set(orVector, 11);
 *
 *     PARCBitVector *result = parcBitVector_Or(parcBitVector, orVector);
 *     assertTrue(parcBitVector_NumberOfBitsSet(result) == 2, "Vector should have been set");
 *
 *     parcBitVector_Release(&parcBitVector);
 *     parcBitVector_Release(&orVector);
 *     parcBitVector_Release(&result);
 * }
 * @endcode
 *
 */
PARCBitVector *parcBitVector_Or(const PARCBitVector *a, const PARCBitVector *b);

/**
 * Set a vector of bits in a vector
 *
 * @param [in] parcBitVector to set bits in
 * @param [in] bitsToSet vector of bits to set
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     PARCBitVector *bitsToSet = parcBitVector_Create();
 *     parcBitVector_Set(bitsToSet, 10);
 *     parcBitVector_SetVector(parcBitVector, bitsToSet);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 1, "Vector should have been set");
 * }
 * @endcode
 *
 */
void parcBitVector_SetVector(PARCBitVector *parcBitVector, const PARCBitVector *bitsToSet);

/**
 * Reset the bits of bit vector to 0
 *
 * @param [in] parcBitVector to set bits in
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     parcBitVector_Set(parcBitVector, 42);
 *     parcBitVector_Reset(parcBitVector);
 *     assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 0, "Vector should have 0 bits set");
 * }
 * @endcode
 *
 */
void parcBitVector_Reset(PARCBitVector *parcBitVector);

/**
 * Clear a bit in a vector
 *
 * @param [in] parcBitVector to clear bit in
 * @param [in] bit in vector to clear
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 1, "Vector should have been set");
 *     parcBitVector_Clear(parcBitVector, 10);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 0, "Vector should have been cleared");
 * }
 * @endcode
 *
 */
void parcBitVector_Clear(PARCBitVector *parcBitVector, unsigned bit);

/**
 * Clear a vector of bits in a vector
 *
 * @param [in] parcBitVector to clear bits in
 * @param [in] bitsToClear vector of bits to clear
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     PARCBitVector *bitsToClear = parcBitVector_Create();
 *     parcBitVector_Set(bitsToClear, 10);
 *     parcBitVector_SetVector(parcBitVector, bitsToClear);
 *     assertTrue(parcBitVector_Get(parcBitVector, 10) == 0, "Vector should have been cleared");
 * }
 * @endcode
 *
 */
void parcBitVector_ClearVector(PARCBitVector *parcBitVector, const PARCBitVector *bitsToClear);

/**
 * Return number of bits set in a vector
 *
 * @param [in] parcBitVector to return number of bits set from
 * @returns number of bits currently set in bit vector
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     assertTrue(parcBitVector_NumberOfBitsSet(parcBitVector) == 1, "One bit should have been set");
 * }
 * @endcode
 *
 */
unsigned parcBitVector_NumberOfBitsSet(const PARCBitVector *parcBitVector);

/**
 * Return index if next set bit in vector
 *
 * @param [in] parcBitVector to inspect
 * @param [in] startFrom bit position to start inspection from
 * @returns index of next bit set in vector after startFrom or -1 if none
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 10);
 *     parcBitVector_Set(parcBitVector, 12);
 *     assertTrue(parcBitVector_NextBitSet(parcBitVector, 0) == 10, "Bit 10 should have been found first");
 *     assertTrue(parcBitVector_NextBitSet(parcBitVector, 11) == 12, "Bit 12 should have been found next");
 * }
 * @endcode
 *
 */
unsigned parcBitVector_NextBitSet(const PARCBitVector *parcBitVector, unsigned startFrom);

/**
 * Return text representation of a bit vector
 *
 * @param [in] parcBitVector to represent
 * @returns allocated character string representing bit vector which must be released by parcMemory_Deallocate
 *
 * Example:
 * @code
 * {
 *     PARCBitVector *parcBitVector = parcBitVector_Create();
 *     parcBitVector_Set(parcBitVector, 1);
 *     const char *bitVectorString = parcBitVector_ToString(parcBitVector);
 *     printf("Vector contents: %s\n", bitVectorString);
 *     parcMemory_Deallocate(&bitVectorString);
 * }
 * @endcode
 *
 */
char *parcBitVector_ToString(const PARCBitVector *parcBitVector);
#endif // libparc_parc_BitVector_h
