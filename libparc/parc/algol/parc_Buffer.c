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
#include <ctype.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_ByteArray.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_HashCode.h>

struct parc_buffer {
    PARCByteArray *array;

    size_t capacity;

    size_t arrayOffset;  // The offset within this buffer's backing PARCByteArray of the first element.

    size_t position;      // The index, relative to arrayOffset, of the next byte to be read from or written to this buffer.

    /**
     * The index, relative to arrayOffset, of the last position that cannot be read or written.
     */
    size_t limit;
    /**
     * A buffer's mark is the index, relative to arrayOffset, to which its position will be set when the reset function is invoked.
     * The mark is not always defined, but when it is defined it is never negative and is never greater than the position.
     * If the mark is defined then it is discarded when the position or the limit is adjusted to a value smaller than the mark.
     * If the mark is not defined then invoking the reset function causes a trap.
     */
    size_t mark;
};

static inline void
_discardMark(PARCBuffer *buffer)
{
    buffer->mark = SIZE_MAX;
}

static inline bool
_markIsDiscarded(const PARCBuffer *buffer)
{
    return buffer->mark == SIZE_MAX;
}

static inline void
_trapIfIndexExceedsLimit(const PARCBuffer *buffer, const size_t index)
{
    //parcTrapOutOfBoundsIf(index > buffer->limit, "PARCBuffer limit at %zd, attempted to access at %zd",
    //                  parcBuffer_Limit(buffer), index);
}

static inline void
_trapIfBufferUnderflow(const PARCBuffer *buffer, const size_t requiredRemaining)
{
    _trapIfIndexExceedsLimit(buffer, buffer->position + requiredRemaining);
}

static inline size_t
_effectiveIndex(const PARCBuffer *buffer, const size_t index)
{
    return buffer->arrayOffset + index;
}

static inline size_t
_effectivePosition(const PARCBuffer *buffer)
{
    return buffer->arrayOffset + parcBuffer_Position(buffer);
}

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define _optionalAssertInvariants(_instance_)
#else
#  define _optionalAssertInvariants(_instance_) _assertInvariants(_instance_)

static inline void
_assertInvariants(const PARCBuffer *buffer)
{
    // 0 <= mark <= position <= limit <= capacity
    parcAssertTrue(0 <= buffer->mark,
               "Expected 0 <= mark (%zd)", buffer->mark);
    parcAssertTrue(_markIsDiscarded(buffer) || buffer->mark <= buffer->position,
               "Expected mark (%zd) <= position (%zd)", buffer->mark, buffer->position);
    parcAssertTrue(buffer->position <= buffer->limit,
               "Expected position (%zd) <= limit (%zd)", buffer->position, buffer->limit);
    parcAssertTrue(buffer->limit <= buffer->capacity,
               "Expected limit (%zd) <= capacity (%zd)", buffer->limit, buffer->capacity);
    parcAssertTrue((buffer->arrayOffset + buffer->capacity) <= parcByteArray_Capacity(buffer->array),
               "Expected (%zd + %zd) <= %zd",
               buffer->arrayOffset, buffer->capacity, parcByteArray_Capacity(buffer->array));
}
#endif

static inline int
_digittoint(char digit)
{
    int result = -1;

    int value = digit - '0';

    if ((unsigned) value < 10) {
        result = value;
    } else {
        value = digit - 'a';
        if ((unsigned) value < 6) {
            result = value + 10;
        } else {
            value = digit - 'A';
            if ((unsigned) value < 6) {
                result = value + 10;
            }
        }
    }

    return result;
}

// This hexadecimal parsing code needs to be fixed - see FogBugz:3598

static char *_hexDigitsUpper = "0123456789ABCDEF";
static char *_hexDigitsLower = "0123456789abcdef";

static inline char
_fromHexDigit(char hexDigit)
{
    for (char i = 0; i < 16; i++) {
        if (hexDigit == _hexDigitsLower[(int) i] || hexDigit == _hexDigitsUpper[(int) i]) {
            return i;
        }
    }
    return -1;
}

static inline uint8_t
_hexByte(const char *hexByte)
{
    uint8_t result = (uint8_t) (_fromHexDigit(hexByte[0]) << 4) | _fromHexDigit(hexByte[1]);
    return result;
}

static inline char *
_parcBuffer_CheckValidity(const PARCBuffer *buffer)
{
    char *result = NULL;

    if (buffer != NULL) {
        if (parcObject_IsValid(buffer)) {
            if (parcByteArray_IsValid(buffer->array)) {
                // 0 <= mark <= position <= limit <= capacity
                if (_markIsDiscarded(buffer) || buffer->mark <= buffer->position) {
                    if (buffer->position <= buffer->limit) {
                        if (buffer->limit <= buffer->capacity) {
                            if ((buffer->arrayOffset + buffer->capacity) <= parcByteArray_Capacity(buffer->array)) {
                                result = NULL;
                            } else {
                                result = "PARCBuffer offset+capacity exceeds the capacity of the underlying PARCByteArray";
                            }
                        } else {
                            result = "PARCBuffer limit exceeds the capacity.";
                        }
                    } else {
                        result = "PARCBuffer position exceeds the limit.";
                    }
                } else {
                    result = "PARCBuffer mark exceeds the current position";
                }
            } else {
                result = "PARCBuffer underlying PARCByteArray is invalid";
            }
        } else {
            result = "PARCBuffer is an invalid PARCObject.";
        }
    } else {
        result = "PARCBuffer is NULL";
    }

    return result;
}

void
parcBuffer_AssertValid(const PARCBuffer *buffer)
{
    char *explanation = _parcBuffer_CheckValidity(buffer);

    parcTrapIllegalValueIf(explanation != NULL, "PARCBuffer@%p %s.", (void *) buffer, explanation);
}

bool
parcBuffer_IsValid(const PARCBuffer *buffer)
{
    bool result = false;

    if (buffer != NULL) {
        if (parcObject_IsValid(buffer)) {
            if (parcByteArray_IsValid(buffer->array)) {
                // 0 <= mark <= position <= limit <= capacity
                if (_markIsDiscarded(buffer) || buffer->mark <= buffer->position) {
                    if (buffer->position <= buffer->limit) {
                        if (buffer->limit <= buffer->capacity) {
                            if ((buffer->arrayOffset + buffer->capacity) <= parcByteArray_Capacity(buffer->array)) {
                                result = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

static bool
_parcBuffer_Destructor(PARCBuffer **bufferPtr)
{
    PARCBuffer *buffer = *bufferPtr;

    parcByteArray_Release(&buffer->array);
    return true;
}

parcObject_Override(PARCBuffer, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcBuffer_Destructor,
                    .copy = (PARCObjectCopy *) parcBuffer_Copy,
                    .toString = (PARCObjectToString *) parcBuffer_ToString,
                    .equals = (PARCObjectEquals *) parcBuffer_Equals,
                    .compare = (PARCObjectCompare *) parcBuffer_Compare,
                    .hashCode = (PARCObjectHashCode *) parcBuffer_HashCode,
                    .display = (PARCObjectDisplay *) parcBuffer_Display);

/**
 * Initialise a parcBuffer instance.
 *
 * The buffer's offset, position, limit and capacity are set to the given values.
 * The mark is made invalid.
 *
 * @return The same pointer as `result`.
 */
static PARCBuffer *
_parcBuffer_Init(PARCBuffer *result, PARCByteArray *array, size_t offset, size_t position, size_t limit, size_t capacity)
{
    result->array = array;
    result->arrayOffset = offset;
    result->position = position;
    result->limit = limit;
    result->capacity = capacity;
    _discardMark(result);

    parcBuffer_OptionalAssertValid(result);

    return result;
}

static inline PARCBuffer *
_parcBuffer_getInstance(void)
{
    PARCBuffer *result = parcObject_CreateInstance(PARCBuffer);

    return result;
}

static inline size_t
_computeNewLimit(size_t oldCapacity, size_t oldLimit, size_t newCapacity)
{
    size_t result = newCapacity;

    bool limitIsAtCapacity = (oldLimit == oldCapacity);

    if (newCapacity > oldCapacity) {
        if (limitIsAtCapacity) {
            result = newCapacity;
        } else {
            result = oldLimit;
        }
    } else {
        if (limitIsAtCapacity) {
            result = newCapacity;
        } else {
            result = (oldLimit < newCapacity) ? oldLimit : newCapacity;
        }
    }

    return result;
}

static inline size_t
_computeNewMark(size_t oldMark, size_t newLimit, size_t newCapacity)
{
    size_t result = oldMark;

    if (oldMark != SIZE_MAX) {
        if (oldMark > newCapacity) {
            result = SIZE_MAX;
        } else {
            result = oldMark;
        }

        if (result > newLimit) {
            result = SIZE_MAX;
        }
    }

    return result;
}

PARCBuffer *
parcBuffer_WrapByteArray(PARCByteArray *byteArray, size_t position, size_t limit)
{
    PARCBuffer *result = NULL;

    // The limit cannot exceed the capacity of the PARCByteArray.
    if (limit > parcByteArray_Capacity(byteArray)) {
        return NULL;
    }
    if (byteArray != NULL) {
        result = _parcBuffer_getInstance();

        if (result != NULL) {
            return _parcBuffer_Init(result,
                                    parcByteArray_Acquire(byteArray),
                                    0, position, limit,
                                    parcByteArray_Capacity(byteArray));
        }
    }

    return result;
}

PARCBuffer *
parcBuffer_Resize(PARCBuffer *buffer, size_t newCapacity)
{
    parcBuffer_OptionalAssertValid(buffer);

    PARCByteArray *newArray = parcByteArray_Allocate(newCapacity);
    if (newArray == NULL) {
        return NULL;
    }

    size_t numberOfBytesToCopy = parcBuffer_Capacity(buffer);
    if (numberOfBytesToCopy > newCapacity) {
        numberOfBytesToCopy = newCapacity;
    }

    parcByteArray_PutBytes(newArray, 0, numberOfBytesToCopy, &parcByteArray_Array(buffer->array)[buffer->arrayOffset]);

    parcByteArray_Release(&buffer->array);

    buffer->array = newArray;
    buffer->arrayOffset = 0;
    buffer->limit = _computeNewLimit(buffer->capacity, buffer->limit, newCapacity);
    buffer->mark = _computeNewMark(buffer->mark, buffer->limit, newCapacity);
    buffer->capacity = newCapacity;
    buffer->position = (buffer->position < buffer->limit) ? buffer->position : buffer->limit;

    parcBuffer_OptionalAssertValid(buffer);

    return buffer;
}

PARCBuffer *
parcBuffer_Allocate(size_t capacity)
{
    PARCByteArray *array = parcByteArray_Allocate(capacity);

    if (array != NULL) {
        PARCBuffer *result = _parcBuffer_getInstance();
        if (result != NULL) {
            return _parcBuffer_Init(result, array, 0, 0, capacity, capacity);
        }
        parcByteArray_Release(&array);
    }

    return NULL;
}

PARCBuffer *
parcBuffer_Wrap(void *array, size_t arrayLength, size_t position, size_t limit)
{
    PARCBuffer *result = NULL;

    if (array != NULL) {
        PARCByteArray *byteArray = parcByteArray_Wrap(arrayLength, array);

        if (byteArray != NULL) {
            result = parcBuffer_WrapByteArray(byteArray, position, limit);
            parcByteArray_Release(&byteArray);
        }
    }

    return result;
}

PARCBuffer *
parcBuffer_WrapCString(char *string)
{
    size_t length = strlen(string);
    return parcBuffer_Wrap(string, length, 0, length);
}

PARCBuffer *
parcBuffer_AllocateCString(const char *string)
{
    size_t length = strlen(string);
    PARCBuffer *buffer = parcBuffer_Allocate(length + 1);
    parcBuffer_PutArray(buffer, length, (const uint8_t *) string);
    parcBuffer_PutUint8(buffer, 0);
    parcBuffer_SetPosition(buffer, buffer->position - 1);
    parcBuffer_Flip(buffer);

    return buffer;
}

PARCBuffer *
parcBuffer_ParseHexString(const char *hexString)
{
    size_t length = strlen(hexString);

    // The hex string must be an even length greater than zero.
    if (length > 0 && (length % 2) == 1) {
        return NULL;
    }

    PARCBuffer *result = parcBuffer_Allocate(length);
    for (size_t i = 0; i < length; i += 2) {
        parcBuffer_PutUint8(result, _hexByte(&hexString[i]));
    }

    return result;
}

PARCBuffer *
parcBuffer_CreateFromArray(const void *bytes, const size_t length)
{
    parcAssertTrue(length == 0 || bytes != NULL,
               "If the byte array is NULL, then length MUST be zero.");

    PARCBuffer *result = parcBuffer_Allocate(length);
    parcBuffer_PutArray(result, length, bytes);

    return result;
}

parcObject_ImplementAcquire(parcBuffer, PARCBuffer);

parcObject_ImplementRelease(parcBuffer, PARCBuffer);

size_t
parcBuffer_Capacity(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->capacity;
}

PARCBuffer *
parcBuffer_Clear(PARCBuffer *buffer)
{
    parcBuffer_SetPosition(buffer, 0);
    parcBuffer_SetLimit(buffer, parcBuffer_Capacity(buffer));
    _discardMark(buffer);
    return buffer;
}

bool
parcBuffer_Equals(const PARCBuffer *x, const PARCBuffer *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    return (parcBuffer_Compare(x, y) == 0);
}

int
parcBuffer_Compare(const PARCBuffer *x, const PARCBuffer *y)
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

    size_t count = parcBuffer_Remaining(x);
    if (count > parcBuffer_Remaining(y)) {
        count = parcBuffer_Remaining(y);
    }

    int result = 0;

    if (count > 0) {
        result = memcmp(parcBuffer_Overlay((PARCBuffer *) x, 0), parcBuffer_Overlay((PARCBuffer *) y, 0), count);
    }

    if (result == 0) {
        // This is when one buffer is longer than the other, and they are equivalent thus far.
        ssize_t difference = parcBuffer_Remaining(x) - parcBuffer_Remaining(y);
        if (difference > 0) {
            result = +1;
        } else if (difference < 0) {
            result = -1;
        }
    }

    return result;
}

PARCByteArray *
parcBuffer_Array(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->array;
}

PARCBuffer *
parcBuffer_Duplicate(const PARCBuffer *original)
{
    PARCBuffer *result = _parcBuffer_getInstance();
    if (result != NULL) {
        _parcBuffer_Init(result,
                         parcByteArray_Acquire(original->array),
                         original->arrayOffset,
                         original->position,
                         parcBuffer_Limit(original),
                         original->capacity);

        _optionalAssertInvariants(result);
    }
    return result;
}

PARCBuffer *
parcBuffer_Slice(const PARCBuffer *original)
{
    PARCBuffer *result = _parcBuffer_getInstance();
    if (result != NULL) {
        _parcBuffer_Init(result,
                         parcByteArray_Acquire(original->array),
                         original->arrayOffset + parcBuffer_Position(original),
                         0,
                         parcBuffer_Limit(original) - parcBuffer_Position(original),
                         parcBuffer_Limit(original) - parcBuffer_Position(original));

        _optionalAssertInvariants(result);
    }
    return result;
}

size_t
parcBuffer_ArrayOffset(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->arrayOffset;
}

PARCBuffer *
parcBuffer_Reset(PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    parcAssertFalse(_markIsDiscarded(buffer),
                "The mark has not been set");
    buffer->position = buffer->mark;

    _optionalAssertInvariants(buffer);

    return buffer;
}

size_t
parcBuffer_Limit(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->limit;
}

PARCBuffer *
parcBuffer_Mark(PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    buffer->mark = buffer->position;
    _optionalAssertInvariants(buffer);
    return buffer;
}

PARCBuffer *
parcBuffer_SetLimit(PARCBuffer *buffer, size_t newLimit)
{
    parcBuffer_OptionalAssertValid(buffer);
    parcAssertTrue(newLimit <= parcBuffer_Capacity(buffer),
               "new limit cannot be larger than the capacity");

    if (_markIsDiscarded(buffer)) {
        buffer->limit = newLimit;
        _discardMark(buffer);
    } else {
        if (newLimit < buffer->position) {
            buffer->position = newLimit;
        }
        if (newLimit < buffer->mark) {
            _discardMark(buffer);
        }
        buffer->limit = newLimit;
    }
    _optionalAssertInvariants(buffer);
    return buffer;
}

size_t
parcBuffer_Position(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->position;
}

size_t
parcBuffer_Remaining(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return buffer->limit - buffer->position;
}

bool
parcBuffer_HasRemaining(const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    return parcBuffer_Remaining(buffer) != 0;
}

PARCBuffer *
parcBuffer_SetPosition(PARCBuffer *buffer, size_t newPosition)
{
    parcBuffer_OptionalAssertValid(buffer);

    parcAssertFalse(newPosition > buffer->limit,
                "new position cannot be greater the buffer's limit");

    buffer->position = newPosition;
    if (!_markIsDiscarded(buffer) && newPosition < buffer->mark) {
        _discardMark(buffer);
    }

    _optionalAssertInvariants(buffer);
    return buffer;
}

PARCBuffer *
parcBuffer_Rewind(PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);

    buffer->position = 0;
    _discardMark(buffer);

    _optionalAssertInvariants(buffer);
    return buffer;
}

PARCBuffer *
parcBuffer_Copy(const PARCBuffer *original)
{
    parcBuffer_OptionalAssertValid(original);

    PARCBuffer *result = _parcBuffer_getInstance();

    if (result != NULL) {
        PARCByteArray *array = parcByteArray_Copy(original->array);
        if (array != NULL) {
            _parcBuffer_Init(result,
                             array,
                             parcBuffer_ArrayOffset(original),
                             parcBuffer_Position(original),
                             parcBuffer_Limit(original),
                             parcBuffer_Capacity(original));
        } else {
            parcBuffer_Release(&result);
        }
    }

    return result;
}

PARCBuffer *
parcBuffer_Flip(PARCBuffer *result)
{
    parcBuffer_OptionalAssertValid(result);

    size_t position = result->position;
    result->position = 0;
    result->limit = position;

    _optionalAssertInvariants(result);

    return result;
}

uint8_t
parcBuffer_GetAtIndex(const PARCBuffer *buffer, size_t index)
{
    parcBuffer_OptionalAssertValid(buffer);

    _trapIfIndexExceedsLimit(buffer, index);

    return parcByteArray_GetByte(buffer->array, _effectiveIndex(buffer, index));
}

void *
parcBuffer_Overlay(PARCBuffer *buffer, size_t length)
{
    parcBuffer_OptionalAssertValid(buffer);
    _trapIfBufferUnderflow(buffer, length);

    uint8_t *result = parcByteArray_AddressOfIndex(buffer->array, _effectiveIndex(buffer, parcBuffer_Position(buffer)));
    buffer->position += length;
    return result;
}

uint8_t
parcBuffer_GetUint8(PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);
    _trapIfBufferUnderflow(buffer, 1);

    uint8_t result = parcByteArray_GetByte(buffer->array, _effectivePosition(buffer));
    buffer->position++;

    return result;
}

PARCBuffer *
parcBuffer_GetBytes(PARCBuffer *buffer, size_t length, uint8_t array[length])
{
    parcBuffer_OptionalAssertValid(buffer);
    _trapIfBufferUnderflow(buffer, length);

    parcByteArray_GetBytes(buffer->array, _effectivePosition(buffer), length, array);
    buffer->position += length;

    return buffer;
}

uint16_t
parcBuffer_GetUint16(PARCBuffer *buffer)
{
    uint8_t high = parcBuffer_GetUint8(buffer);
    uint8_t low = parcBuffer_GetUint8(buffer);

    uint16_t result = (high << 8) | low;

    return result;
}

uint32_t
parcBuffer_GetUint32(PARCBuffer *buffer)
{
    uint32_t result = 0;
    for (int i = 0; i < sizeof(uint32_t); i++) {
        result = result << 8 | parcBuffer_GetUint8(buffer);
    }
    return result;
}

uint64_t
parcBuffer_GetUint64(PARCBuffer *buffer)
{
    uint64_t result = 0;
    for (int i = 0; i < sizeof(uint64_t); i++) {
        result = result << 8 | parcBuffer_GetUint8(buffer);
    }
    return result;
}

PARCBuffer *
parcBuffer_PutUint8(PARCBuffer *buffer, uint8_t value)
{
    parcBuffer_OptionalAssertValid(buffer);
    parcAssertTrue(parcBuffer_Remaining(buffer) >= 1,
               "Buffer overflow");

    parcByteArray_PutByte(buffer->array, _effectivePosition(buffer), value);
    buffer->position++;
    return buffer;
}

PARCBuffer *
parcBuffer_PutUint16(PARCBuffer *buffer, uint16_t value)
{
    parcAssertTrue(parcBuffer_Remaining(buffer) >= sizeof(uint16_t),
               "Buffer overflow");

    parcBuffer_PutUint8(buffer, (value >> 8) & 0xFF);
    parcBuffer_PutUint8(buffer, value & 0xFF);
    return buffer;
}

PARCBuffer *
parcBuffer_PutUint32(PARCBuffer *buffer, uint32_t value)
{
    parcAssertTrue(parcBuffer_Remaining(buffer) >= sizeof(uint32_t),
               "Buffer overflow");
    for (int i = sizeof(uint32_t) - 1; i > 0; i--) {
        uint8_t b = value >> (i * 8) & 0xFF;
        parcBuffer_PutUint8(buffer, b);
    }
    parcBuffer_PutUint8(buffer, value & 0xFF);
    return buffer;
}

PARCBuffer *
parcBuffer_PutUint64(PARCBuffer *buffer, uint64_t value)
{
    parcAssertTrue(parcBuffer_Remaining(buffer) >= sizeof(uint64_t),
               "Buffer overflow");
    for (int i = sizeof(uint64_t) - 1; i > 0; i--) {
        uint8_t b = value >> (i * 8) & 0xFF;
        parcBuffer_PutUint8(buffer, b);
    }
    parcBuffer_PutUint8(buffer, value & 0xFF);
    return buffer;
}

PARCBuffer *
parcBuffer_PutAtIndex(PARCBuffer *buffer, size_t index, uint8_t value)
{
    parcBuffer_OptionalAssertValid(buffer);
    parcAssertTrue(_effectiveIndex(buffer, index) < parcBuffer_Limit(buffer), "Buffer overflow");

    parcByteArray_PutByte(buffer->array, _effectiveIndex(buffer, index), value);
    return buffer;
}

PARCBuffer *
parcBuffer_PutArray(PARCBuffer *buffer, size_t arrayLength, const uint8_t array[arrayLength])
{
    parcBuffer_OptionalAssertValid(buffer);
    parcAssertTrue(parcBuffer_Remaining(buffer) >= arrayLength,
               "Buffer overflow");

    parcByteArray_PutBytes(buffer->array, _effectivePosition(buffer), arrayLength, array);
    return parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + arrayLength);
}

PARCBuffer *
parcBuffer_PutCString(PARCBuffer *buffer, const char *string)
{
    return parcBuffer_PutArray(buffer, strlen(string) + 1, (const uint8_t *) string);
}

PARCBuffer *
parcBuffer_PutBuffer(PARCBuffer *result, const PARCBuffer *buffer)
{
    parcBuffer_OptionalAssertValid(buffer);
    parcAssertTrue(parcBuffer_Remaining(result) >= parcBuffer_Remaining(buffer),
               "Buffer overflow. %zd bytes remaining, %zd required.", parcBuffer_Remaining(result), parcBuffer_Remaining(buffer));

    size_t length = parcBuffer_Remaining(buffer);
    parcByteArray_ArrayCopy(result->array, _effectivePosition(result), buffer->array, _effectivePosition(buffer), length);
    parcBuffer_SetPosition(result, parcBuffer_Position(result) + length);
    return result;
}

PARCHashCode
parcBuffer_HashCode(const PARCBuffer *buffer)
{
    PARCHashCode result = 0;

    size_t remaining = parcBuffer_Remaining(buffer);
    if (remaining > 0) {
        result = parcHashCode_Hash(parcBuffer_Overlay((PARCBuffer *) buffer, 0), parcBuffer_Remaining(buffer));
    }
    return result;
}

size_t
parcBuffer_FindUint8(const PARCBuffer *buffer, uint8_t byte)
{
    for (size_t i = parcBuffer_Position(buffer); i < parcBuffer_Limit(buffer); i++) {
        if (parcBuffer_GetAtIndex(buffer, i) == byte) {
            return i;
        }
    }
    return SIZE_MAX;
}

char *
parcBuffer_ToString(const PARCBuffer *buffer)
{
    size_t remaining = parcBuffer_Remaining(buffer);

    char *result = parcMemory_Allocate(remaining + 1);
    if (remaining > 0) {
        parcAssertNotNull(result, "parcMemory_Allocate returned NULL");
        if (result != NULL) {
            memcpy(result, parcBuffer_Overlay((PARCBuffer *) buffer, 0), remaining);
        }
    }
    result[remaining] = 0;
    return result;
}

void
parcBuffer_Display(const PARCBuffer *buffer, int indentation)
{
    if (buffer == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCBuffer@NULL");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCBuffer@%p {", (void *) buffer);
        parcDisplayIndented_PrintLine(indentation + 1,
                                      ".arrayOffset=%zd .position=%zd .limit=%zd .mark=%zd",
                                      buffer->arrayOffset, buffer->position, buffer->limit, buffer->mark);
        parcByteArray_Display(buffer->array, indentation + 1);
        parcDisplayIndented_PrintLine(indentation, "}");
    }
}

// Given a value, return the low nibble as a hex character.
static char
_toHexDigit(const char value)
{
    return "0123456789ABCDEF"[value & 0xF];
}

char *
parcBuffer_ToHexString(const PARCBuffer *buffer)
{
    if (buffer == NULL) {
        return parcMemory_StringDuplicate("null", 4);
    }

    size_t length = parcBuffer_Remaining(buffer);
    // Hopefully length is less than (2^(sizeof(size_t)*8) / 2)

    char *result = parcMemory_AllocateAndClear((length * 2) + 1);
    parcAssertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", (length * 2) + 1);

    for (size_t i = 0; i < length; i++) {
        unsigned char byte = parcBuffer_GetAtIndex(buffer, i);
        result[i * 2] = _toHexDigit(byte >> 4);
        result[i * 2 + 1] = _toHexDigit(byte);
    }
    result[length * 2] = 0;

    return result;
}

bool
parcBuffer_SkipOver(PARCBuffer *buffer, size_t length, const uint8_t bytesToSkipOver[length])
{
    while (parcBuffer_Remaining(buffer) > 0) {
        uint8_t character = parcBuffer_GetUint8(buffer);
        if (memchr(bytesToSkipOver, character, length) == NULL) {
            parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) - 1);
            return true;
        }
    }
    return false;
}

bool
parcBuffer_SkipTo(PARCBuffer *buffer, size_t length, const uint8_t bytesToSkipTo[length])
{
    bool result = false;

    while (parcBuffer_Remaining(buffer) > 0) {
        uint8_t character = parcBuffer_GetUint8(buffer);
        if (memchr(bytesToSkipTo, character, length) != NULL) {
            parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) - 1);
            result = true;
            break;
        }
    }
    return result;
}

uint8_t
parcBuffer_PeekByte(const PARCBuffer *buffer)
{
    return parcBuffer_GetAtIndex(buffer, parcBuffer_Position(buffer));
}

uint64_t
parcBuffer_ParseHexNumber(PARCBuffer *buffer)
{
    char *bytes = parcBuffer_Overlay(buffer, 0);

    int start = 0;
    if (parcBuffer_Remaining(buffer) > 2) {
        if (bytes[0] == '0' && bytes[1] == 'x') {
            start = 2;
        }
    }

    unsigned count = 0;
    uint64_t result = 0;
    for (int i = start; i < parcBuffer_Remaining(buffer) && isxdigit(bytes[i]); i++) {
        result = (result * 16) + _digittoint(bytes[i]);
        count++;
    }

    parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + start + count);

    return result;
}

uint64_t
parcBuffer_ParseDecimalNumber(PARCBuffer *buffer)
{
    char *bytes = parcBuffer_Overlay(buffer, 0);

    int start = 0;

    unsigned count = 0;
    uint64_t result = 0;
    for (int i = start; i < parcBuffer_Remaining(buffer) && isdigit(bytes[i]); i++) {
        result = (result * 10) + _digittoint(bytes[i]);
        count++;
    }

    parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + count);

    return result;
}

uint64_t
parcBuffer_ParseNumeric(PARCBuffer *buffer)
{
    uint64_t result = 0;

    char *bytes = parcBuffer_Overlay(buffer, 0);

    if (parcBuffer_Remaining(buffer) > 2 && bytes[0] == '0' && bytes[1] == 'x') {
        result = parcBuffer_ParseHexNumber(buffer);
    } else {
        result = parcBuffer_ParseDecimalNumber(buffer);
    }

    return result;
}
