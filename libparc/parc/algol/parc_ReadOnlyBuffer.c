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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_ReadOnlyBuffer.h>
#include <parc/algol/parc_DisplayIndented.h>

struct parc_readonly_buffer {
    PARCBuffer *buffer;
};

static void
_readOnlyBuffer_Finalize(PARCReadOnlyBuffer **bufferPtr)
{
    PARCReadOnlyBuffer *buffer = *bufferPtr;

    parcBuffer_Release(&buffer->buffer);
}

parcObject_ExtendPARCObject(PARCReadOnlyBuffer, _readOnlyBuffer_Finalize, parcReadOnlyBuffer_Copy,
                            parcReadOnlyBuffer_ToString, parcReadOnlyBuffer_Equals, NULL, parcReadOnlyBuffer_HashCode, NULL);

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Create(PARCBuffer *buffer)
{
    PARCReadOnlyBuffer *result = NULL;
    if (buffer != NULL) {
        result = parcObject_CreateInstance(PARCReadOnlyBuffer);
        if (result != NULL) {
            result->buffer = parcBuffer_WrapByteArray(parcBuffer_Array(buffer), parcBuffer_Position(buffer), parcBuffer_Limit(buffer));
        }
    }
    return result;
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Wrap(uint8_t *array, size_t arrayLength, size_t position, size_t limit)
{
    PARCReadOnlyBuffer *result = parcObject_CreateInstance(PARCReadOnlyBuffer);
    if (result != NULL) {
        result->buffer = parcBuffer_Wrap(array, arrayLength, position, limit);
    }
    return result;
}

parcObject_ImplementAcquire(parcReadOnlyBuffer, PARCReadOnlyBuffer);

parcObject_ImplementRelease(parcReadOnlyBuffer, PARCReadOnlyBuffer);

size_t
parcReadOnlyBuffer_Capacity(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_Capacity(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Clear(PARCReadOnlyBuffer *buffer)
{
    parcBuffer_Clear(buffer->buffer);
    return buffer;
}

bool
parcReadOnlyBuffer_Equals(const PARCReadOnlyBuffer *x, const PARCReadOnlyBuffer *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    return parcBuffer_Equals(x->buffer, y->buffer);
}

PARCByteArray *
parcReadOnlyBuffer_Array(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_Array(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Copy(const PARCReadOnlyBuffer *original)
{
    PARCReadOnlyBuffer *result = parcObject_CreateInstance(PARCReadOnlyBuffer);

    if (result != NULL) {
        result->buffer = parcBuffer_Copy(original->buffer);
    }
    return result;
}

size_t
parcReadOnlyBuffer_ArrayOffset(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_ArrayOffset(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Rewind(PARCReadOnlyBuffer *buffer)
{
    parcBuffer_Rewind(buffer->buffer);
    return buffer;
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Reset(PARCReadOnlyBuffer *buffer)
{
    parcBuffer_Reset(buffer->buffer);
    return buffer;
}

size_t
parcReadOnlyBuffer_Limit(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_Limit(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Mark(PARCReadOnlyBuffer *buffer)
{
    parcBuffer_Mark(buffer->buffer);
    return buffer;
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_SetLimit(PARCReadOnlyBuffer *buffer, size_t newLimit)
{
    parcBuffer_SetLimit(buffer->buffer, newLimit);

    return buffer;
}

void *
parcReadOnlyBuffer_Overlay(PARCReadOnlyBuffer *buffer, size_t length)
{
    return parcBuffer_Overlay(buffer->buffer, length);
}

size_t
parcReadOnlyBuffer_Position(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_Position(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_SetPosition(PARCReadOnlyBuffer *buffer, size_t newPosition)
{
    parcBuffer_SetPosition(buffer->buffer, newPosition);
    return buffer;
}

size_t
parcReadOnlyBuffer_Remaining(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_Remaining(buffer->buffer);
}

bool
parcReadOnlyBuffer_HasRemaining(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_HasRemaining(buffer->buffer);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_Flip(PARCReadOnlyBuffer *buffer)
{
    parcBuffer_Flip(buffer->buffer);
    return buffer;
}

uint8_t
parcReadOnlyBuffer_GetAtIndex(const PARCReadOnlyBuffer *buffer, size_t index)
{
    return parcBuffer_GetAtIndex(buffer->buffer, index);
}

PARCReadOnlyBuffer *
parcReadOnlyBuffer_GetArray(PARCReadOnlyBuffer *buffer, uint8_t *array, size_t length)
{
    parcBuffer_GetBytes(buffer->buffer, length, array);
    return buffer;
}

uint8_t
parcReadOnlyBuffer_GetUint8(PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_GetUint8(buffer->buffer);
}

uint16_t
parcReadOnlyBuffer_GetUint16(PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_GetUint16(buffer->buffer);
}

uint32_t
parcReadOnlyBuffer_GetUint32(PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_GetUint32(buffer->buffer);
}

uint64_t
parcReadOnlyBuffer_GetUint64(PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_GetUint64(buffer->buffer);
}

PARCHashCode
parcReadOnlyBuffer_HashCode(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_HashCode(buffer->buffer);
}

char *
parcReadOnlyBuffer_ToString(const PARCReadOnlyBuffer *buffer)
{
    return parcBuffer_ToString(buffer->buffer);
}

void
parcReadOnlyBuffer_Display(const PARCReadOnlyBuffer *buffer, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCReadOnlyBuffer@%p {\n", (void *) buffer);
    parcBuffer_Display(buffer->buffer, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}\n");
}
