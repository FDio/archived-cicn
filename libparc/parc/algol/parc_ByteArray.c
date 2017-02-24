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
 */
#include <config.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_ByteArray.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_DisplayIndented.h>

struct parc_byte_array {
    uint8_t *array;
    size_t length;
    void (*freeFunction)(void **);
};
#define MAGIC 0x0ddba11c1a55e5

static inline void
_trapIfOutOfBounds(const PARCByteArray *array, const size_t index)
{
    trapOutOfBoundsIf(index >= array->length, "parcByteArray index %zd exceeds the length %zd", index, array->length);
}

static bool
_parcByteArray_Destructor(PARCByteArray **byteArrayPtr)
{
    PARCByteArray *byteArray = *byteArrayPtr;

    if (byteArray->freeFunction != NULL) {
        if (byteArray->array != NULL) {
            byteArray->freeFunction((void **) &(byteArray->array));
        }
    }
    return true;
}

parcObject_Override(PARCByteArray, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcByteArray_Destructor,
                    .copy = (PARCObjectCopy *) parcByteArray_Copy,
                    .equals = (PARCObjectEquals *) parcByteArray_Equals,
                    .compare = (PARCObjectCompare *) parcByteArray_Compare,
                    .hashCode = (PARCObjectHashCode *) parcByteArray_HashCode,
                    .display = (PARCObjectDisplay *) parcByteArray_Display);

void
parcByteArray_AssertValid(const PARCByteArray *instance)
{
    trapInvalidValueIf(parcByteArray_IsValid(instance) == false,
                       "PARCByteArray instance is invalid.");
}

bool
parcByteArray_IsValid(const PARCByteArray *instance)
{
    bool result = false;

    if (instance != NULL) {
        if (instance->length == 0 || instance->array != NULL) {
            result = true;
        }
    }

    return result;
}

PARCByteArray *
parcByteArray_Allocate(const size_t length)
{
    uint8_t *array = NULL;
    if (length > 0) {
        array = parcMemory_AllocateAndClear(sizeof(uint8_t) * length);
        if (array == NULL) {
            return NULL;
        }
    }
    PARCByteArray *result = parcObject_CreateInstance(PARCByteArray);

    if (result != NULL) {
        result->array = array;
        result->length = length;
        result->freeFunction = parcMemory_DeallocateImpl;
        return result;
    } else {
        parcMemory_Deallocate(&array);
    }
    return NULL;
}

PARCByteArray *
parcByteArray_Wrap(const size_t length, uint8_t array[length])
{
    if (array != NULL) {
        PARCByteArray *result = parcObject_CreateInstance(PARCByteArray);
        if (result != NULL) {
            result->array = array;
            result->length = length;
            result->freeFunction = NULL;
            return result;
        }
    }
    return NULL;
}

parcObject_ImplementAcquire(parcByteArray, PARCByteArray);

parcObject_ImplementRelease(parcByteArray, PARCByteArray);

PARCByteArray *
parcByteArray_Copy(const PARCByteArray *original)
{
    parcByteArray_OptionalAssertValid(original);

    PARCByteArray *result = NULL;

    if (original != NULL) {
        result = parcByteArray_Allocate(original->length);
        memcpy(result->array, original->array, result->length);
    }

    return result;
}

size_t
parcByteArray_Capacity(const PARCByteArray *byteArray)
{
    parcByteArray_OptionalAssertValid(byteArray);
    return byteArray->length;
}

PARCByteArray *
parcByteArray_PutByte(PARCByteArray *result, size_t index, uint8_t byte)
{
    parcByteArray_OptionalAssertValid(result);
    _trapIfOutOfBounds(result, index);

    result->array[index] = byte;
    return result;
}

uint8_t
parcByteArray_GetByte(const PARCByteArray *array, size_t index)
{
    parcByteArray_OptionalAssertValid(array);
    _trapIfOutOfBounds(array, index);

    return array->array[index];
}

uint8_t *
parcByteArray_Array(const PARCByteArray *byteArray)
{
    parcByteArray_OptionalAssertValid(byteArray);
    return byteArray->array;
}

uint8_t *
parcByteArray_AddressOfIndex(const PARCByteArray *array, const size_t index)
{
    parcByteArray_OptionalAssertValid(array);
    _trapIfOutOfBounds(array, index);

    return &array->array[index];
}

int
parcByteArray_Compare(const PARCByteArray *x, const PARCByteArray *y)
{
    if (x == y) {
        return 0;
    }

    if (x == NULL) {
        return -1;
    }

    if (y == NULL) {
        return +1;
    }

    if (parcByteArray_Capacity(x) > parcByteArray_Capacity(y)) {
        return +1;
    }
    if (parcByteArray_Capacity(x) < parcByteArray_Capacity(y)) {
        return -1;
    }

    return memcmp(x->array, y->array, parcByteArray_Capacity(x));
}

PARCByteArray *
parcByteArray_PutBytes(PARCByteArray *result, size_t offset, size_t length, const uint8_t source[length])
{
    parcByteArray_OptionalAssertValid(result);
    trapOutOfBoundsIf(offset > result->length,
                      "The offset (%zd) exeeds the length (%zd) of the PARCByteArray.", offset, result->length);

    size_t available = result->length - offset;

    trapOutOfBoundsIf(length > available, "%zd available bytes, %zd required.", available, length);

    memcpy(&result->array[offset], source, length);
    return result;
}

PARCByteArray *
parcByteArray_GetBytes(const PARCByteArray *result, size_t offset, size_t length, uint8_t array[length])
{
    parcByteArray_OptionalAssertValid(result);

    size_t available = result->length - offset;

    trapOutOfBoundsIf(length > available, "parcByteArray_CopyOut %zd available bytes, %zd required", available, length);

    memcpy(array, &result->array[offset], length);
    return (PARCByteArray *) result;
}

PARCByteArray *
parcByteArray_ArrayCopy(PARCByteArray *destination, size_t destOffset, const PARCByteArray *source, size_t srcOffset, size_t length)
{
    parcByteArray_OptionalAssertValid(destination);
    parcByteArray_OptionalAssertValid(source);

    memcpy(&destination->array[destOffset], &source->array[srcOffset], length);
    return destination;
}

bool
parcByteArray_Equals(const PARCByteArray *a, const PARCByteArray *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    if (a->length == b->length) {
        return memcmp(a->array, b->array, a->length) == 0;
    }
    return false;
}

static void
_parcByteArray_PrettyPrintLine(const unsigned char *memory, size_t offset, size_t length)
{
    int bytesPerLine = 16;
    char accumulator[bytesPerLine + 1];
    memset(accumulator, ' ', bytesPerLine);
    accumulator[bytesPerLine] = 0;

    printf("%5zd: ", offset);

    for (int i = 0; i < bytesPerLine; i++) {
        if (offset + i >= length) {
            printf("      ");
            accumulator[i] = ' ';
        } else {
            char c = memory[offset + i];
            printf("0x%02x, ", c & 0xFF);
            if (isprint(c)) {
                accumulator[i] = c;
            } else {
                accumulator[i] = '.';
            }
        }
    }
    printf("   %s\n", accumulator);
}

void
parcByteArray_Display(const PARCByteArray *array, int indentation)
{
    int bytesPerLine = 16;

    if (array->array == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCByteArray@NULL");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCByteArray@%p = [0,%zd)", (void *) array, array->length);

        for (size_t offset = 0; offset < array->length; offset += bytesPerLine) {
            _parcByteArray_PrettyPrintLine(array->array, offset, array->length);
        }
    }
}

PARCHashCode
parcByteArray_HashCode(const PARCByteArray *array)
{
    parcByteArray_OptionalAssertValid(array);
    return parcHashCode_Hash(array->array, array->length);
}
