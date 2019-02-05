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

#include <config.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_BitVector.h>
#include <parc/algol/parc_Memory.h>

#define BITS_PER_BYTE 8
#define DEFAULT_BITARRAY_SIZE 1
#define MAX_BIT_VECTOR_INDEX 8192

struct PARCBitVector {
    // the number of bits allocated.
    unsigned bitLength;

    // we track the number of "1"s set for fast computation
    // <code>parcBitVector_NumberOfBitsSet()</code>
    unsigned numberOfBitsSet;

    // optimize case where only one bit is set.
    unsigned firstBitSet;

    // our backing memory.
    uint8_t *bitArray;

    // Fill value to be used when extending a vector
    int fillValue;
};

static void
_destroy(PARCBitVector **parcBitVector)
{
    parcMemory_Deallocate(&((*parcBitVector)->bitArray));
}

parcObject_ExtendPARCObject(PARCBitVector, _destroy, parcBitVector_Copy, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(parcBitVector, PARCBitVector);

parcObject_ImplementRelease(parcBitVector, PARCBitVector);

PARCBitVector *
parcBitVector_Create(void)
{
    PARCBitVector *parcBitVector = parcObject_CreateInstance(PARCBitVector);
    parcAssertNotNull(parcBitVector, "parcObject_CreateInstance returned NULL");

    parcBitVector->bitArray = parcMemory_AllocateAndClear(DEFAULT_BITARRAY_SIZE);
    parcAssertNotNull(parcBitVector->bitArray, "parcMemory_AllocateAndClear(%d) returned NULL", DEFAULT_BITARRAY_SIZE);
    parcBitVector->bitLength = DEFAULT_BITARRAY_SIZE * BITS_PER_BYTE;
    parcBitVector->numberOfBitsSet = 0;
    parcBitVector->firstBitSet = -1;
    parcBitVector->fillValue = 0;

    return parcBitVector;
}

PARCBitVector *
parcBitVector_Copy(const PARCBitVector *source)
{
    PARCBitVector *parcBitVector = parcObject_CreateInstance(PARCBitVector);
    parcAssertNotNull(parcBitVector, "parcObject_CreateInstance returned NULL");

    size_t byteLength = source->bitLength / BITS_PER_BYTE;
    parcBitVector->bitArray = parcMemory_Allocate(byteLength);
    memcpy(parcBitVector->bitArray, source->bitArray, byteLength);
    parcBitVector->bitLength = source->bitLength;
    parcBitVector->numberOfBitsSet = source->numberOfBitsSet;
    parcBitVector->firstBitSet = source->firstBitSet;
    parcBitVector->fillValue = source->fillValue;

    return parcBitVector;
}

bool
parcBitVector_Equals(const PARCBitVector *a, const PARCBitVector *b)
{
    bool equal = ((a->numberOfBitsSet == b->numberOfBitsSet) &&
                  (a->firstBitSet == b->firstBitSet));

    if (equal) {
        int byteLength = a->bitLength / BITS_PER_BYTE;
        if ((a->bitLength != b->bitLength) &&
            (b->bitLength < a->bitLength)) {
            byteLength = b->bitLength / BITS_PER_BYTE;
        }
        equal = (memcmp(a->bitArray, b->bitArray, byteLength) == 0);
    }

    return equal;
}

static void
_parc_bit_vector_resize(PARCBitVector *parcBitVector, unsigned bit)
{
    parcAssertNotNull(parcBitVector, "_parc_bit_vector_resize passed a NULL parcBitVector");
    parcAssertTrue(bit < MAX_BIT_VECTOR_INDEX, "_parc_bit_vector_resize passed a bit index that's too large");

    unsigned neededBits = bit + 1;
    if (neededBits > parcBitVector->bitLength) {
        int newSize = (neededBits / BITS_PER_BYTE) + ((neededBits % BITS_PER_BYTE) ? 1 : 0);
        int oldSize = (parcBitVector->bitLength / BITS_PER_BYTE) + ((parcBitVector->bitLength % BITS_PER_BYTE) ? 1 : 0);

        uint8_t *newArray = parcMemory_Reallocate(parcBitVector->bitArray, newSize);
        parcAssertTrue(newArray, "parcMemory_Reallocate(%d) failed", newSize);
        // Reallocate does not guarantee that additional memory is zero-filled.
        memset((void *) &(newArray[oldSize]), parcBitVector->fillValue, newSize - oldSize);

        parcBitVector->bitArray = newArray;
        parcAssertNotNull(newArray, "parcMemory_Reallocate(%d) returned NULL", newSize);
        parcBitVector->bitLength = newSize * BITS_PER_BYTE;
    }
}

int
parcBitVector_Get(const PARCBitVector *parcBitVector, unsigned bit)
{
    parcAssertTrue(bit < MAX_BIT_VECTOR_INDEX, "parcBitVector_Set passed a bit index that's too large");

    if ((parcBitVector == NULL) || (bit >= parcBitVector->bitLength)) {
        return -1;
    }

    int byte = bit / BITS_PER_BYTE;
    int bitInByte = bit % BITS_PER_BYTE;
    if ((parcBitVector->bitArray[byte] & (1 << bitInByte))) {
        return 1;
    }

    return 0;
}

void
parcBitVector_Set(PARCBitVector *parcBitVector, unsigned bit)
{
    parcAssertNotNull(parcBitVector, "parcBitVector_Set passed a NULL parcBitVector");
    parcAssertTrue(bit < MAX_BIT_VECTOR_INDEX, "parcBitVector_Set passed a bit index that's too huge");
    if (bit >= parcBitVector->bitLength) {
        _parc_bit_vector_resize(parcBitVector, bit);
    }
    int byte = bit / BITS_PER_BYTE;
    int bitInByte = bit % BITS_PER_BYTE;
    if (!(parcBitVector->bitArray[byte] & (1 << bitInByte))) {
        parcBitVector->bitArray[byte] |= (1 << bitInByte);
        parcBitVector->numberOfBitsSet++;
    }
    if ((parcBitVector->firstBitSet == -1) || (bit < parcBitVector->firstBitSet)) {
        parcBitVector->firstBitSet = bit;
    }
}

void
parcBitVector_SetVector(PARCBitVector *parcBitVector, const PARCBitVector *bitsToSet)
{
    parcAssertNotNull(parcBitVector, "parcBitVector_SetVector passed a NULL parcBitVector");
    parcAssertNotNull(parcBitVector, "parcBitVector_SetVector passed a NULL vector of bits to set");
    int settingBit = 0;
    for (unsigned int i = 0; i < bitsToSet->numberOfBitsSet; i++) {
        settingBit = parcBitVector_NextBitSet(bitsToSet, settingBit);
        parcAssertTrue(settingBit != -1, "Number of bits claimed set inconsistent with bits found");
        parcBitVector_Set(parcBitVector, settingBit);
        settingBit++;
    }
}

void
parcBitVector_Reset(PARCBitVector *parcBitVector)
{
    parcBitVector->numberOfBitsSet = 0;
    parcBitVector->firstBitSet = -1;
    parcBitVector->fillValue = 0;
    memset(parcBitVector->bitArray, parcBitVector->fillValue, parcBitVector->bitLength / BITS_PER_BYTE);
}


void
parcBitVector_Clear(PARCBitVector *parcBitVector, unsigned bit)
{
    parcAssertNotNull(parcBitVector, "parcBitVector_Clear passed a NULL parcBitVector");
    parcAssertTrue(bit < MAX_BIT_VECTOR_INDEX, "parcBitVector_Clear passed a bit index that's too huge");
    if (bit > parcBitVector->bitLength) {
        _parc_bit_vector_resize(parcBitVector, bit);
    }
    int byte = bit / BITS_PER_BYTE;
    int bitInByte = bit % BITS_PER_BYTE;
    if (parcBitVector->bitArray[byte] & (1 << bitInByte)) {
        parcBitVector->bitArray[byte] &= ~(1 << bitInByte);
        parcBitVector->numberOfBitsSet--;
    }
    if (bit == parcBitVector->firstBitSet) {
        parcBitVector->firstBitSet = parcBitVector_NextBitSet(parcBitVector, bit + 1);
    }
}

void
parcBitVector_ClearVector(PARCBitVector *parcBitVector, const PARCBitVector *bitsToClear)
{
    parcAssertNotNull(parcBitVector, "parcBitVector_ClearVector passed a NULL parcBitVector");
    parcAssertNotNull(bitsToClear, "parcBitVector_ClearVector passed a NULL for vector of bitsToClear");

    // If we're clearing ourself, we just need a reset
    if (parcBitVector == bitsToClear) {
        parcBitVector_Reset(parcBitVector);
        return;
    }

    unsigned int clearingBit = 0;
    for (unsigned int i = 0; i < bitsToClear->numberOfBitsSet; i++) {
        clearingBit = parcBitVector_NextBitSet(bitsToClear, clearingBit);
        // only clear up to the end of the original vector
        if (clearingBit >= parcBitVector->bitLength) {
            return;
        }
        if (clearingBit != -1) {
            parcBitVector_Clear(parcBitVector, clearingBit);
        }
        clearingBit++;
    }
}

unsigned
parcBitVector_NumberOfBitsSet(const PARCBitVector *parcBitVector)
{
    return parcBitVector->numberOfBitsSet;
}

unsigned
parcBitVector_NextBitSet(const PARCBitVector *parcBitVector, unsigned startFrom)
{
    if (startFrom >= parcBitVector->bitLength) {
        return -1;
    }
    if (startFrom <= parcBitVector->firstBitSet) {
        return parcBitVector->firstBitSet;
    }
    int byte = startFrom / BITS_PER_BYTE;
    int bitInByte = startFrom % BITS_PER_BYTE;
    int allocatedBytes = ((parcBitVector->bitLength) / BITS_PER_BYTE);
    for (; byte < allocatedBytes; byte++) {
        if (parcBitVector->bitArray[byte]) {
            for (; bitInByte < BITS_PER_BYTE; bitInByte++) {
                if (parcBitVector->bitArray[byte] & (1 << bitInByte)) {
                    return (byte * BITS_PER_BYTE) + bitInByte;
                }
            }
        }
        bitInByte = 0;
    }
    return -1;
}

bool
parcBitVector_Contains(const PARCBitVector *parcBitVector, const PARCBitVector *testVector)
{
    bool result = true;

    int testBit = 0;
    for (unsigned int i = 0; i < testVector->numberOfBitsSet; i++, testBit++) {
        testBit = parcBitVector_NextBitSet(testVector, testBit);
        if (parcBitVector_Get(parcBitVector, testBit) != 1) {
            result = false;
            break;
        }
    }

    return result;
}

char *
parcBitVector_ToString(const PARCBitVector *parcBitVector)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        int nextBitSet = 0;
        parcBufferComposer_Format(composer, "[ ");
        for (int index = parcBitVector_NumberOfBitsSet(parcBitVector); index; index--) {
            nextBitSet = parcBitVector_NextBitSet(parcBitVector, nextBitSet);
            parcBufferComposer_Format(composer, "%d ", nextBitSet);
            nextBitSet++;
        }
        parcBufferComposer_Format(composer, "]");
        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        parcBufferComposer_Release(&composer);

        result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);
    }

    return result;
}

PARCBitVector *
parcBitVector_Or(const PARCBitVector *first, const PARCBitVector *second)
{
    PARCBitVector *result = NULL;

    if (first != NULL) {
        result = parcBitVector_Copy(first);
        if (second != NULL) {
            for (int bit = 0; (bit = parcBitVector_NextBitSet(second, bit)) >= 0; bit++) {
                parcBitVector_Set(result, bit);
            }
        }
    } else if (second != NULL) {
        result = parcBitVector_Copy(second);
    } else { // both null, or is empty
        result = parcBitVector_Create();
    }

    return result;
}

PARCBitVector *
parcBitVector_And(const PARCBitVector *first, const PARCBitVector *second)
{
    PARCBitVector *result = parcBitVector_Create();

    if (second == NULL) {
        if (first != NULL) {
            return result;
        }
    }

    if (first != NULL) {
        for (int bit = 0; (bit = parcBitVector_NextBitSet(first, bit)) >= 0; bit++) {
            if (parcBitVector_Get(second, bit) == 1) {
                parcBitVector_Set(result, bit);
            }
        }
    }

    return result;
}

static PARCBitVector *
_parcBitVector_LeftShiftOnce(PARCBitVector *parcBitVector)
{
    if (parcBitVector != NULL) {
        for (int bit = 0; (bit = parcBitVector_NextBitSet(parcBitVector, bit)) >= 0; bit++) {
            if (bit > 0) { // The first bit falls off
                parcBitVector_Set(parcBitVector, bit - 1);
            }
            parcBitVector_Clear(parcBitVector, bit);
        }
    }

    return parcBitVector;
}

PARCBitVector *
parcBitVector_LeftShift(PARCBitVector *parcBitVector, size_t count)
{
    for (int i = 0; i < count; i++) {
        _parcBitVector_LeftShiftOnce(parcBitVector);
    }

    return parcBitVector;
}

static PARCBitVector *
_parcBitVector_RightShiftOnce(PARCBitVector *parcBitVector)
{
    if (parcBitVector != NULL) {
        for (int bit = 0; (bit = parcBitVector_NextBitSet(parcBitVector, bit)) >= 0; bit++) {
            // Shift the next sequence of one bits into the first zero bit
            int nextZero = bit + 1;
            while (parcBitVector_Get(parcBitVector, nextZero) == 1) {
                nextZero++;
            }
            parcBitVector_Clear(parcBitVector, bit++);
            while (bit <= nextZero) {
                parcBitVector_Set(parcBitVector, bit);
                bit++;
            }
        }
    }

    return parcBitVector;
}

PARCBitVector *
parcBitVector_RightShift(PARCBitVector *parcBitVector, size_t count)
{
    for (int i = 0; i < count; i++) {
        _parcBitVector_RightShiftOnce(parcBitVector);
    }

    return parcBitVector;
}
